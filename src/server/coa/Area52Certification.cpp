#include "Area52CertificationRules.h"
#include "AscensionFreePick.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Item.h"
#include "Mail.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "WorldSessionMgr.h"
#include <boost/bind/placeholders.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <chrono>
#include <mutex>

namespace Area52Certification
{
namespace
{
std::map<std::uint32_t, bool> Entries;
std::map<std::uint32_t, std::vector<std::uint32_t>> GrantedRanks;
std::map<std::uint32_t, TesterScore> Scores;
std::map<std::uint32_t, std::chrono::steady_clock::time_point> Requests;
std::mutex Mutex;
bool Active = false;
std::string Evidence;

void Send(Player* player, std::string const& payload)
{
    WorldPacket packet;
    ChatHandler::BuildChatPacket(packet, CHAT_MSG_WHISPER, LANG_ADDON, player, player,
        std::string(Prefix) + payload);
    player->SendDirectMessage(&packet);
}

void Reward(Player* player)
{
    if (!Active || !AscensionFreePick::Applies(player) || player->GetSession()->IsBot())
        return;
    auto const account = player->GetSession()->GetAccountId();
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_CERTIFICATION_REWARD);
    query->SetData(0, account);
    auto result = CharacterDatabase.Query(query);
    if (!result)
        return;
    auto const itemId = result->Fetch()[0].Get<uint32>();
    Item* item = Item::CreateItem(itemId, 1);
    if (!item)
        return;
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* claim = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_CERTIFICATION_REWARD);
    claim->SetData(0, account);
    claim->SetData(1, player->GetGUID().GetCounter());
    claim->SetData(2, itemId);
    transaction->Append(claim);
    item->SaveToDB(transaction);
    MailDraft("Area 52 Certification Milestone",
        "Thank you for testing and certifying Area 52 abilities, talents, and Mystic Enchants. "
        "Your account has reached its certification milestone. Enjoy your Hoplite Vanity Bundle!")
        .AddItem(item).SendMailTo(transaction, MailReceiver(player), MailSender(player));
    if (!CharacterDatabase.AsyncCommitTransaction(transaction).m_future.get())
        LOG_ERROR("coa", "Area 52 certification reward transaction failed for account {}", account);
}

void SendLeaderboard(Player* player, std::uint32_t requestedPage)
{
    std::map<std::uint32_t, TesterScore> accounts;
    std::map<std::uint32_t, std::string> names;
    auto* characters = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_LAST_CHARACTERS);
    if (auto result = CharacterDatabase.Query(characters))
        do
        {
            auto const fields = result->Fetch();
            names.emplace(fields[0].Get<std::uint32_t>(), fields[1].Get<std::string>());
        } while (result->NextRow());
    auto* statement = LoginDatabase.GetPreparedStatement(LOGIN_SEL_A52_LEADERBOARD_ACCOUNTS);
    if (auto result = LoginDatabase.Query(statement))
        do
        {
            auto const fields = result->Fetch();
            auto const account = fields[0].Get<std::uint32_t>();
            auto const found = names.find(account);
            if (found == names.end())
                continue;
            auto const& name = found->second;
            auto const score = Scores.find(account);
            accounts.emplace(account, TesterScore{account, name, score == Scores.end() ? 0 : score->second.Total, 0});
        } while (result->NextRow());
    auto const rows = Leaderboard(accounts);
    auto const pages = std::max<std::size_t>(1, (rows.size() + 9) / 10);
    auto const page = std::min<std::size_t>(requestedPage, pages);
    Send(player, "LB|" + std::to_string(page) + "|" + std::to_string(pages) + "|" + std::to_string(rows.size()));
    for (auto i = (page - 1) * 10; i < std::min(page * 10, rows.size()); ++i)
    {
        auto const& row = rows[i];
        Send(player, "LR|" + std::to_string(row.Rank) + "|" + row.Name + "|" + std::to_string(row.Total) +
            "|" + (row.Account == player->GetSession()->GetAccountId() ? "1" : "0"));
    }
    Send(player, "LE");
}

void Snapshot(Player* player)
{
    Send(player, "BEGIN");
    for (bool certified : {false, true})
    {
        std::string packet = certified ? "C" : "V";
        for (auto const& [id, value] : Entries)
        {
            if (value != certified)
                continue;
            auto const token = "|" + std::to_string(id);
            if (packet.size() + token.size() > 220)
            {
                Send(player, packet);
                packet = certified ? "C" : "V";
            }
            packet += token;
        }
        if (packet.size() > 1)
            Send(player, packet);
    }
    Send(player, "END");
}

class CertificationWorld final : public WorldScript
{
public:
    CertificationWorld() : WorldScript("Area52CertificationWorld") { }

    void OnStartup() override
    {
        if (!AscensionFreePick::Enabled() ||
            !sConfigMgr->GetOption<bool>("CoA.FreePick.TesterCertification", false))
            return;
        try
        {
            boost::property_tree::ptree report;
            boost::property_tree::read_json(sConfigMgr->GetOption<std::string>(
                "CoA.FreePick.CertificationFile", ""), report);
            GrantedRanks = ReadGrantedRanks(report);
            Entries = ReadEligibility(report);
            Evidence = report.get<std::string>("evidence_sha256", "");
            auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_CERTIFICATIONS);
            if (auto result = CharacterDatabase.Query(statement))
                do
                {
                    auto const fields = result->Fetch();
                    Credit(Scores, fields[1].Get<std::uint32_t>(), fields[2].Get<std::string>());
                    auto found = Entries.find(fields[0].Get<std::uint32_t>());
                    if (found != Entries.end())
                        found->second = true;
                } while (result->NextRow());
            Active = !Entries.empty();
            LOG_INFO("coa", "Area 52 tester certification loaded: {} eligible entries", Entries.size());
        }
        catch (std::exception const& error)
        {
            Entries.clear();
            GrantedRanks.clear();
            LOG_ERROR("coa", "Area 52 tester certification disabled: {}", error.what());
        }
    }
};

class CertificationPlayer final : public PlayerScript
{
public:
    CertificationPlayer() : PlayerScript("Area52CertificationPlayer",
        {PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_LOGIN}) { }

    void OnPlayerLogin(Player* player) override
    {
        std::lock_guard lock(Mutex);
        if (Active && AscensionFreePick::Applies(player))
        {
            auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_UPS_A52_LAST_CHARACTER);
            statement->SetData(0, player->GetSession()->GetAccountId());
            statement->SetData(1, player->GetGUID().GetCounter());
            statement->SetData(2, player->GetName());
            CharacterDatabase.DirectExecute(statement);
        }
        Reward(player);
    }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& message,
        Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER || language != LANG_ADDON || !message.starts_with(Prefix))
            return true;
        if (receiver != player)
            return false;
        std::lock_guard lock(Mutex);
        if (!Active || !AscensionFreePick::Applies(player))
        {
            Send(player, "ERROR|Certification is unavailable on this realm.");
            return false;
        }
        auto const account = player->GetSession()->GetAccountId();
        auto const now = std::chrono::steady_clock::now();
        auto found = Requests.find(account);
        if (found != Requests.end() && now - found->second < std::chrono::seconds(2))
        {
            Send(player, "ERROR|Please wait a moment and try again.");
            return false;
        }
        Requests[account] = now;
        std::string_view const payload(message.data() + Prefix.size(), message.size() - Prefix.size());
        if (payload == "SYNC")
        {
            Snapshot(player);
            return false;
        }
        if (auto const page = ParseLeaderboardPage(payload))
        {
            SendLeaderboard(player, page);
            return false;
        }
        auto const id = ParseConfirmation(payload);
        auto entry = Entries.find(id);
        if (!id || entry == Entries.end())
        {
            Send(player, "ERROR|Only VERIFIED abilities, talents, or Mystic Enchants can be certified.");
            return false;
        }
        if (auto const grants = GrantedRanks.find(id); grants != GrantedRanks.end())
            if (std::none_of(grants->second.begin(), grants->second.end(),
                [&](std::uint32_t spell) { return player->HasSpell(spell); }))
            {
                Send(player, "ERROR|Learn this granted ability before certifying it.");
                return false;
            }
        if (!entry->second)
        {
            auto* insert = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_CERTIFICATION);
            insert->SetData(0, id);
            insert->SetData(1, account);
            insert->SetData(2, player->GetGUID().GetCounter());
            insert->SetData(3, player->GetName());
            insert->SetData(4, Evidence);
            CharacterDatabase.DirectExecute(insert);
            auto* select = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_CERTIFICATION);
            select->SetData(0, id);
            auto saved = CharacterDatabase.Query(select);
            if (!saved)
            {
                Send(player, "ERROR|Certification could not be saved. Please try again.");
                return false;
            }
            Credit(Scores, saved->Fetch()[1].Get<std::uint32_t>(), saved->Fetch()[2].Get<std::string>());
            entry->second = true;
            for (auto const& [sessionId, session] : sWorldSessionMgr->GetAllSessions())
                if (auto* online = session->GetPlayer(); online && online->IsInWorld() &&
                    AscensionFreePick::Applies(online))
                    Send(online, "C|" + std::to_string(id));
        }
        Reward(player);
        Send(player, "OK|" + std::to_string(id));
        return false;
    }

    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard lock(Mutex);
        Requests.erase(player->GetSession()->GetAccountId());
    }
};
}
}

void AddSC_Area52Certification()
{
    new Area52Certification::CertificationWorld();
    new Area52Certification::CertificationPlayer();
}
