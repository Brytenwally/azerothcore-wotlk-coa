#include "Area52BuildProtocol.h"
#include "AscensionCompatOpcodes.h"
#include "AscensionFreePick.h"
#include "Config.h"
#include "CryptoHash.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace
{
using namespace Area52Build;
using Clock = std::chrono::steady_clock;
std::atomic<bool> Enabled{false};

struct SessionState
{
    WorldSession* Owner = nullptr;
    std::atomic<bool> Active{true};
    std::atomic<bool> Busy{false};
    std::atomic<bool> OwnedReady{false};
    std::deque<WorldPacket> Requests;
    std::deque<WorldPacket> Responses;
    uint32 ResponseDelay = 0;
    Clock::time_point NextRequest{};
};

std::mutex StateLock;
std::unordered_map<uint32, std::shared_ptr<SessionState>> Sessions;

std::string Hex(std::vector<uint8> const& bytes)
{
    std::string result;
    result.reserve(bytes.size() * 2);
    for (uint8 value : bytes)
    {
        result += "0123456789abcdef"[value >> 4];
        result += "0123456789abcdef"[value & 15];
    }
    return result;
}

bool Unhex(std::string const& hex, Entry& entry)
{
    if (hex.size() % 2 || hex.size() > MaxPacketBytes * 2)
        return false;
    std::vector<uint8> bytes;
    bytes.reserve(hex.size() / 2);
    for (std::size_t i = 0; i < hex.size(); i += 2)
    {
        unsigned value = 0;
        auto result = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
        if (result.ec != std::errc{} || result.ptr != hex.data() + i + 2)
            return false;
        bytes.push_back(uint8(value));
    }
    return Decode(bytes, entry);
}

bool Id(std::string const& text, uint64& id)
{
    auto result = std::from_chars(text.data(), text.data() + text.size(), id);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size() && id != 0;
}

void Append(WorldPacket& packet, Entry const& entry)
{
    auto bytes = Encode(entry);
    packet.append(bytes.data(), bytes.size());
}

bool Current(WorldSession* session, std::shared_ptr<SessionState> const& state)
{
    return Enabled && state->Active && state->Owner == session && session->GetPlayer()
        && AscensionFreePick::Applies(session->GetPlayer());
}

void PublishResult(WorldSession* session, uint16 opcode, char const* result, Entry const& entry)
{
    WorldPacket response(opcode + 1, 128);
    response << result;
    if (std::string_view(result) == "PUBLISH_BUILD_OK")
        Append(response, entry);
    else if (opcode == Save)
        response << entry.Id;
    session->SendPacket(&response);
}

void SendOwned(WorldSession* session, std::shared_ptr<SessionState> state)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD_OWNED);
    query->SetData(0, session->GetAccountId());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state](PreparedQueryResult rows)
        {
            if (!Current(session, state))
                return;
            WorldPacket packet(0x634, 128);
            packet << uint32(rows ? rows->GetRowCount() : 0);
            if (rows)
                do
                    packet << std::to_string(rows->Fetch()[0].Get<uint64>());
                while (rows->NextRow());
            session->SendPacket(&packet);
            state->OwnedReady = true;
        }));
}

bool ReadRecord(Field* fields, Entry& entry)
{
    if (!Unhex(fields[2].Get<std::string>(), entry))
        return false;
    entry.Flags = fields[3].Get<uint8>() ? 1 : 0;
    entry.NeedsRepairs = 0;
    return true;
}

void Browse(WorldSession* session, std::shared_ptr<SessionState> state, uint32 category)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD_LIST);
    query->SetData(0, session->GetAccountId());
    query->SetData(1, category);
    query->SetData(2, category);
    query->SetData(3, session->GetAccountId());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state](PreparedQueryResult rows)
        {
            if (!Current(session, state))
                return;
            std::vector<Entry> entries;
            if (rows)
                do
                {
                    Entry entry;
                    if (ReadRecord(rows->Fetch(), entry))
                        entries.push_back(std::move(entry));
                } while (rows->NextRow());
            std::vector<std::vector<uint8>> pagesData(1);
            std::vector<uint32> counts(1, 0);
            for (auto const& entry : entries)
            {
                auto bytes = Encode(entry);
                if (pagesData.back().size() + bytes.size() > 48000)
                {
                    pagesData.emplace_back();
                    counts.push_back(0);
                }
                pagesData.back().insert(pagesData.back().end(), bytes.begin(), bytes.end());
                ++counts.back();
            }
            uint32 const pages = uint32(pagesData.size());
            std::lock_guard lock(StateLock);
            for (uint32 page = 0; page < pages; ++page)
            {
                WorldPacket packet(0x62F, 128);
                packet << "QUERY_BUILDS_OK" << page << pages << counts[page];
                if (!pagesData[page].empty())
                    packet.append(pagesData[page].data(), pagesData[page].size());
                state->Responses.push_back(std::move(packet));
            }
        }));
}

void GetBuild(WorldSession* session, std::shared_ptr<SessionState> state, uint64 id)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD_PUBLIC);
    query->SetData(0, id);
    query->SetData(1, session->GetAccountId());
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, id](PreparedQueryResult rows)
        {
            state->Busy = false;
            if (!Current(session, state))
                return;
            Entry entry;
            bool const found = rows && ReadRecord(rows->Fetch(), entry);
            if (!found)
                entry.Id = std::to_string(id);
            WorldPacket packet(0x631, 128);
            packet << (found ? "QUERY_BUILD_OK" : "QUERY_BUILD_NOT_FOUND");
            Append(packet, entry);
            session->SendPacket(&packet);
        }));
}

void Commit(WorldSession* session, std::shared_ptr<SessionState> state, uint16 opcode,
    Entry entry, uint64 id, uint32 previous)
{
    entry.Id = std::to_string(id);
    entry.Class = 10;
    entry.Upvotes = 0;
    entry.Flags = 0;
    entry.NeedsRepairs = 0;
    entry.Updated = std::max<uint64>(GameTime::GetGameTime().count(), entry.Updated + 1);
    if (opcode == Create)
    {
        entry.Created = entry.Updated;
        entry.Author = session->GetPlayer()->GetName();
    }
    auto const bytes = Encode(entry);
    if (bytes.size() > MaxPacketBytes)
    {
        state->Busy = false;
        PublishResult(session, opcode, "PUBLISH_BUILD_UNKNOWN", entry);
        return;
    }
    std::string const wire = Hex(bytes);
    auto transaction = CharacterDatabase.BeginTransaction();
    if (opcode == Create)
    {
        auto* header = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_BUILD);
        header->SetData(0, id);
        header->SetData(1, session->GetAccountId());
        transaction->Append(header);
    }
    auto* revision = CharacterDatabase.GetPreparedStatement(CHAR_INS_A52_BUILD_REVISION);
    revision->SetData(0, previous + 1);
    revision->SetData(1, entry.Category);
    revision->SetData(2, entry.Name);
    revision->SetData(3, wire);
    revision->SetData(4, id);
    revision->SetData(5, session->GetAccountId());
    revision->SetData(6, previous);
    transaction->Append(revision);
    session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
        [session, state, opcode, entry, wire, id](bool success)
        {
            if (!Current(session, state))
                return;
            if (!success)
            {
                state->Busy = false;
                PublishResult(session, opcode, "PUBLISH_BUILD_UNKNOWN", entry);
                return;
            }
            auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD);
            query->SetData(0, id);
            session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
                [session, state, opcode, entry, wire](PreparedQueryResult result)
                {
                    state->Busy = false;
                    if (!Current(session, state))
                        return;
                    bool const stored = result && result->Fetch()[2].Get<std::string>() == wire;
                    PublishResult(session, opcode, stored ? "PUBLISH_BUILD_OK" : "PUBLISH_BUILD_UNKNOWN", entry);
                }));
        });
}

char const* Validate(Entry const& entry)
{
    if (entry.Name.empty())
        return "PUBLISH_BUILD_NO_NAME";
    if (entry.Spells.empty())
        return "PUBLISH_BUILD_NO_SPELLS";
    if (entry.Category < 1 || entry.Category > 7)
        return "PUBLISH_BUILD_BAD_CATEGORY";
    if (entry.Difficulty > 4 || entry.PrimaryStat > 6 || entry.Roles > 15)
        return "PUBLISH_BUILD_UNKNOWN";
    for (auto const& spell : entry.Spells)
        if (!sSpellMgr->GetSpellInfo(spell.Id) || spell.Level < 1 || spell.Level > 80)
            return "PUBLISH_BUILD_LEVEL_INAPPROPRIATE_SPELL";
    for (auto const& enchant : entry.Enchants)
        if (!sSpellMgr->GetSpellInfo(enchant.Id) || enchant.Level < 1 || enchant.Level > 80)
            return "PUBLISH_BUILD_UNKNOWN";
    return nullptr;
}

void Publish(WorldSession* session, std::shared_ptr<SessionState> state, WorldPacket const& packet)
{
    Entry entry;
    if (!Decode(std::span(packet.contents(), packet.size()), entry))
    {
        state->Busy = false;
        PublishResult(session, packet.GetOpcode(), "PUBLISH_BUILD_UNKNOWN", entry);
        return;
    }
    if (char const* error = Validate(entry))
    {
        state->Busy = false;
        PublishResult(session, packet.GetOpcode(), error, entry);
        return;
    }
    if (packet.GetOpcode() == Create)
    {
        if (!entry.Id.empty())
        {
            state->Busy = false;
            PublishResult(session, Create, "PUBLISH_BUILD_NO_PERMISSION", entry);
            return;
        }
        auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD_COUNT);
        entry.Author.clear();
        entry.Class = 10;
        entry.Created = 0;
        entry.Updated = 0;
        entry.Upvotes = 0;
        entry.Flags = 0;
        entry.NeedsRepairs = 0;
        auto const digest = Acore::Crypto::SHA256::GetDigestOf(
            std::to_string(session->GetAccountId()) + ":" + Hex(Encode(entry)));
        uint64 id = 0;
        for (unsigned i = 0; i < 8; ++i)
            id = (id << 8) | digest[i];
        id = (id & 0x7FFFFFFFFFFFFFFFULL) | 1;
        query->SetData(0, id);
        query->SetData(1, session->GetAccountId());
        session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
            [session, state, entry, id](PreparedQueryResult rows) mutable
            {
                if (!Current(session, state))
                    return;
                if (!rows || rows->Fetch()[0].Get<uint64>() >= 128)
                {
                    state->Busy = false;
                    PublishResult(session, Create, "PUBLISH_BUILD_UNKNOWN", entry);
                    return;
                }
                if (rows->Fetch()[1].Get<uint64>())
                {
                    state->Busy = false;
                    PublishResult(session, Create, "PUBLISH_BUILD_ALREADY_EXISTS", entry);
                    return;
                }
                Commit(session, state, Create, std::move(entry), id, 0);
            }));
        return;
    }
    uint64 id = 0;
    if (!Id(entry.Id, id))
    {
        state->Busy = false;
        PublishResult(session, Save, "PUBLISH_BUILD_NOT_FOUND", entry);
        return;
    }
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_BUILD);
    query->SetData(0, id);
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, entry, id](PreparedQueryResult rows) mutable
        {
            if (!Current(session, state))
                return;
            Entry previous;
            char const* error = nullptr;
            if (!rows || !ReadRecord(rows->Fetch(), previous))
                error = "PUBLISH_BUILD_NOT_FOUND";
            else if (rows->Fetch()[0].Get<uint32>() != session->GetAccountId())
                error = "PUBLISH_BUILD_NO_PERMISSION";
            else if (previous.Updated != entry.Updated || rows->Fetch()[1].Get<uint32>() >= 1000)
                error = "PUBLISH_BUILD_UNKNOWN";
            if (error)
            {
                state->Busy = false;
                PublishResult(session, Save, error, entry);
                return;
            }
            entry.Author = previous.Author;
            entry.Created = previous.Created;
            Commit(session, state, Save, std::move(entry), id, rows->Fetch()[1].Get<uint32>());
        }));
}

void DeleteResult(WorldSession* session, std::shared_ptr<SessionState> const& state, char const* result)
{
    state->Busy = false;
    WorldPacket response(Delete + 1, 64);
    response << result;
    session->SendPacket(&response);
}

void DeleteBuild(WorldSession* session, std::shared_ptr<SessionState> state, uint64 id)
{
    auto* query = CharacterDatabase.GetPreparedStatement(CHAR_SEL_A52_DELETE_STATE);
    query->SetData(0, id);
    session->GetQueryProcessor().AddCallback(CharacterDatabase.AsyncQuery(query).WithPreparedCallback(
        [session, state, id](PreparedQueryResult row)
        {
            if (!Current(session, state))
                return;
            if (!row || row->Fetch()[0].Get<uint32>() != session->GetAccountId())
            {
                DeleteResult(session, state, "DELETE_BUILD_UNKNOWN");
                return;
            }
            auto transaction = CharacterDatabase.BeginTransaction();
            auto* update = CharacterDatabase.GetPreparedStatement(CHAR_UPD_A52_ARCHIVE_BUILD);
            update->SetData(0, id);
            update->SetData(1, session->GetAccountId());
            transaction->Append(update);
            session->AddTransactionCallback(CharacterDatabase.AsyncCommitTransaction(transaction)).AfterComplete(
                [session, state](bool success)
                {
                    if (!Current(session, state))
                        return;
                    if (success)
                        SendOwned(session, state);
                    DeleteResult(session, state, success ? "DELETE_BUILD_OK" : "DELETE_BUILD_UNKNOWN");
                });
        }));
}

bool Queue(WorldSession* session, WorldPacket const& packet)
{
    if (!Enabled || !session)
        return false;
    std::lock_guard lock(StateLock);
    auto it = Sessions.find(session->GetAccountId());
    if (it != Sessions.end() && it->second->Owner == session
        && packet.size() <= MaxPacketBytes)
    {
        auto& requests = it->second->Requests;
        if (packet.GetOpcode() == List)
        {
            auto pending = std::find_if(requests.begin(), requests.end(), [](WorldPacket const& request)
                { return request.GetOpcode() == List; });
            if (pending != requests.end())
            {
                *pending = packet;
                return true;
            }
        }
        if (requests.size() < 2)
            requests.push_back(packet);
    }
    return true;
}

class BuildWorld final : public WorldScript
{
public:
    BuildWorld() : WorldScript("Area52BuildWorld", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.FreePickBuilds.Enable", false)
            && sConfigMgr->GetOption<bool>("CoA.Enable", true)
            && sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero"
            && sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live"
            && sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) == 0;
    }
};

class BuildPlayer final : public PlayerScript
{
public:
    BuildPlayer() : PlayerScript("Area52BuildPlayer",
        { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_UPDATE }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!Enabled || !AscensionFreePick::Applies(player))
            return;
        auto state = std::make_shared<SessionState>();
        state->Owner = player->GetSession();
        {
            std::lock_guard lock(StateLock);
            auto& previous = Sessions[player->GetSession()->GetAccountId()];
            if (previous)
                previous->Active = false;
            previous = state;
        }
        SendOwned(player->GetSession(), state);
    }

    void OnPlayerLogout(Player* player) override
    {
        std::lock_guard lock(StateLock);
        auto it = Sessions.find(player->GetSession()->GetAccountId());
        if (it != Sessions.end() && it->second->Owner == player->GetSession())
        {
            it->second->Active = false;
            Sessions.erase(it);
        }
    }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!Enabled || !AscensionFreePick::Applies(player))
            return;
        std::shared_ptr<SessionState> state;
        WorldPacket packet;
        {
            std::lock_guard lock(StateLock);
            auto it = Sessions.find(player->GetSession()->GetAccountId());
            if (it == Sessions.end() || it->second->Owner != player->GetSession())
                return;
            state = it->second;
            if (!state->Responses.empty())
            {
                if (state->ResponseDelay > diff)
                {
                    state->ResponseDelay -= diff;
                    return;
                }
                player->GetSession()->SendPacket(&state->Responses.front());
                state->Responses.pop_front();
                state->ResponseDelay = 100;
                state->Busy = !state->Responses.empty();
                return;
            }
            if (!state->OwnedReady || state->Busy || state->Requests.empty() || Clock::now() < state->NextRequest)
                return;
            state->NextRequest = Clock::now() + std::chrono::seconds(2);
            packet = std::move(state->Requests.front());
            state->Requests.pop_front();
            state->Busy = true;
        }
        WorldSession* session = player->GetSession();
        if (packet.GetOpcode() == Create || packet.GetOpcode() == Save)
        {
            Publish(session, state, packet);
            return;
        }
        Reader input(std::span(packet.contents(), packet.size()));
        std::string text;
        if (packet.GetOpcode() == Delete)
        {
            uint64 id = 0;
            if (input.Text(text, 64) && input.Finished() && Id(text, id))
                DeleteBuild(session, state, id);
            else
                DeleteResult(session, state, "DELETE_BUILD_UNKNOWN");
            return;
        }
        if (input.Text(text, 64) && input.Finished())
        {
            if (packet.GetOpcode() == Get)
            {
                uint64 id = 0;
                if (Id(text, id))
                {
                    GetBuild(session, state, id);
                    return;
                }
            }
            else
            {
                std::array<std::string_view, 8> const categories = { "None", "Leveling", "Level60PvE",
                    "Level60PvP", "Level60PvPvE", "Level70PvE", "Level70PvP", "Level70PvPvE" };
                auto it = std::find(categories.begin(), categories.end(), text);
                if (it != categories.end())
                {
                    Browse(session, state, uint32(it - categories.begin()));
                    return;
                }
            }
        }
        if (packet.GetOpcode() == Get)
            GetBuild(session, state, 0);
        else
            Browse(session, state, UINT32_MAX);
    }
};
}

void AddAscensionArea52BuildScripts()
{
    for (uint16 opcode : { Area52Build::Create, Area52Build::Save, Area52Build::Delete, Area52Build::List, Area52Build::Get })
        AscensionCompatOpcodes::Claim(opcode, &Queue);
    new BuildWorld();
    new BuildPlayer();
}
