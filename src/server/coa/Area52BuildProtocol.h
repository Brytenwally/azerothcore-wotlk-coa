#ifndef AREA52_BUILD_PROTOCOL_H
#define AREA52_BUILD_PROTOCOL_H

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Area52Build
{
constexpr std::size_t MaxPacketBytes = 24576;
constexpr std::uint16_t Create = 0x626;
constexpr std::uint16_t Save = 0x628;
constexpr std::uint16_t Delete = 0x62C;
constexpr std::uint16_t List = 0x62E;
constexpr std::uint16_t Get = 0x630;

struct Spell
{
    std::uint32_t Id = 0;
    std::uint32_t Level = 0;
    std::array<std::uint8_t, 4> Roles{};
    std::uint32_t Flags = 0;
    std::string Comment;
};

struct Enchant
{
    std::uint32_t Id = 0;
    std::uint32_t Stacks = 0;
    std::uint32_t Level = 0;
    std::uint32_t Flags = 0;
    std::string Comment;
};

struct EquipmentType
{
    std::uint32_t Type = 0;
    std::string Comment;
};

struct Entry
{
    std::string Id;
    std::array<std::string, 3> Related;
    std::uint32_t Class = 0;
    std::string Author;
    std::string Name;
    std::string Subtext;
    std::string Description;
    std::string Icon;
    std::uint32_t Upvotes = 0;
    std::uint64_t Created = 0;
    std::uint64_t Updated = 0;
    std::uint32_t Category = 0;
    std::uint32_t Roles = 0;
    std::uint32_t PrimaryStat = 0;
    std::vector<Spell> Spells;
    std::vector<Enchant> Enchants;
    std::array<std::vector<EquipmentType>, 2> Equipment;
    std::uint8_t NeedsRepairs = 0;
    std::uint32_t Flags = 0;
    std::uint32_t Difficulty = 0;
};

class Reader
{
public:
    explicit Reader(std::span<std::uint8_t const> bytes) : _bytes(bytes) { }

    template<class T> bool Number(T& value)
    {
        if (_bytes.size() - _position < sizeof(T))
            return false;
        value = 0;
        for (std::size_t index = 0; index < sizeof(T); ++index)
            value |= T(_bytes[_position++]) << (index * 8);
        return true;
    }

    bool Text(std::string& value, std::size_t limit)
    {
        std::size_t const start = _position;
        while (_position < _bytes.size() && _bytes[_position] && _position - start <= limit)
            ++_position;
        if (_position == _bytes.size() || _position - start > limit)
            return false;
        value.assign(reinterpret_cast<char const*>(_bytes.data() + start), _position - start);
        ++_position;
        return true;
    }

    bool Finished() const { return _position == _bytes.size(); }

private:
    std::span<std::uint8_t const> _bytes;
    std::size_t _position = 0;
};

inline bool Decode(std::span<std::uint8_t const> bytes, Entry& output)
{
    if (bytes.size() > MaxPacketBytes)
        return false;
    Entry entry;
    Reader input(bytes);
    if (!input.Text(entry.Id, 64))
        return false;
    for (std::string& id : entry.Related)
        if (!input.Text(id, 64))
            return false;
    if (!input.Number(entry.Class) || !input.Text(entry.Author, 128) || !input.Text(entry.Name, 128)
        || !input.Text(entry.Subtext, 512) || !input.Text(entry.Description, 8192) || !input.Text(entry.Icon, 256)
        || !input.Number(entry.Upvotes) || !input.Number(entry.Created) || !input.Number(entry.Updated)
        || !input.Number(entry.Category) || !input.Number(entry.Roles) || !input.Number(entry.PrimaryStat))
        return false;
    std::uint32_t count = 0;
    if (!input.Number(count) || count > 512)
        return false;
    std::unordered_set<std::uint32_t> spells;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        Spell spell;
        if (!input.Number(spell.Id) || !input.Number(spell.Level) || !spell.Id || !spells.insert(spell.Id).second)
            return false;
        for (std::uint8_t& role : spell.Roles)
            if (!input.Number(role) || role > 1)
                return false;
        if (!input.Number(spell.Flags) || !input.Text(spell.Comment, 128))
            return false;
        entry.Spells.push_back(std::move(spell));
    }
    if (!input.Number(count) || count > 64)
        return false;
    std::unordered_set<std::uint32_t> enchants;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        Enchant enchant;
        if (!input.Number(enchant.Id) || !input.Number(enchant.Stacks) || !input.Number(enchant.Level)
            || !input.Number(enchant.Flags) || !input.Text(enchant.Comment, 128)
            || !enchant.Id || !enchants.insert(enchant.Id).second || !enchant.Stacks || enchant.Stacks > 17)
            return false;
        entry.Enchants.push_back(std::move(enchant));
    }
    for (auto& equipment : entry.Equipment)
    {
        if (!input.Number(count) || count > 32)
            return false;
        std::unordered_set<std::uint32_t> types;
        for (std::uint32_t index = 0; index < count; ++index)
        {
            EquipmentType type;
            if (!input.Number(type.Type) || !input.Text(type.Comment, 128) || !types.insert(type.Type).second)
                return false;
            equipment.push_back(std::move(type));
        }
    }
    if (!input.Number(entry.NeedsRepairs) || entry.NeedsRepairs > 1
        || !input.Number(entry.Flags) || !input.Number(entry.Difficulty) || !input.Finished())
        return false;
    output = std::move(entry);
    return true;
}

class Writer
{
public:
    template<class T> void Number(T value)
    {
        for (std::size_t index = 0; index < sizeof(T); ++index)
            Bytes.push_back(std::uint8_t(value >> (index * 8)));
    }

    void Text(std::string const& value)
    {
        Bytes.insert(Bytes.end(), value.begin(), value.end());
        Bytes.push_back(0);
    }

    std::vector<std::uint8_t> Bytes;
};

inline std::vector<std::uint8_t> Encode(Entry const& entry)
{
    Writer out;
    out.Text(entry.Id);
    for (auto const& id : entry.Related)
        out.Text(id);
    out.Number(entry.Class);
    for (auto const* value : { &entry.Author, &entry.Name, &entry.Subtext, &entry.Description, &entry.Icon })
        out.Text(*value);
    out.Number(entry.Upvotes);
    out.Number(entry.Created);
    out.Number(entry.Updated);
    out.Number(entry.Category);
    out.Number(entry.Roles);
    out.Number(entry.PrimaryStat);
    out.Number(std::uint32_t(entry.Spells.size()));
    for (auto const& spell : entry.Spells)
    {
        out.Number(spell.Id);
        out.Number(spell.Level);
        for (auto role : spell.Roles)
            out.Number(role);
        out.Number(spell.Flags);
        out.Text(spell.Comment);
    }
    out.Number(std::uint32_t(entry.Enchants.size()));
    for (auto const& enchant : entry.Enchants)
    {
        out.Number(enchant.Id);
        out.Number(enchant.Stacks);
        out.Number(enchant.Level);
        out.Number(enchant.Flags);
        out.Text(enchant.Comment);
    }
    for (auto const& equipment : entry.Equipment)
    {
        out.Number(std::uint32_t(equipment.size()));
        for (auto const& type : equipment)
        {
            out.Number(type.Type);
            out.Text(type.Comment);
        }
    }
    out.Number(entry.NeedsRepairs);
    out.Number(entry.Flags);
    out.Number(entry.Difficulty);
    return std::move(out.Bytes);
}
}

#endif
