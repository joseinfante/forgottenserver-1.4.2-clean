// Copyright 2022 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "guild.h"

#include "game.h"

extern Game g_game;

void Guild::addMember(Player* player)
{
	membersOnline.push_back(player);
	for (Player* member : membersOnline) {
		g_game.updatePlayerHelpers(*member);
	}
}

void Guild::removeMember(Player* player)
{
	membersOnline.remove(player);
	for (Player* member : membersOnline) {
		g_game.updatePlayerHelpers(*member);
	}
	g_game.updatePlayerHelpers(*player);

	if (membersOnline.empty()) {
		g_game.removeGuild(id);
		delete this;
	}
}

GuildRank_ptr Guild::getRankById(uint32_t rankId)
{
	for (auto rank : ranks) {
		if (rank->id == rankId) {
			return rank;
		}
	}
	return nullptr;
}

GuildRank_ptr Guild::getRankByName(const std::string& name) const
{
	for (auto rank : ranks) {
		if (rank->name == name) {
			return rank;
		}
	}
	return nullptr;
}

GuildRank_ptr Guild::getRankByLevel(uint8_t level) const
{
	for (auto rank : ranks) {
		if (rank->level == level) {
			return rank;
		}
	}
	return nullptr;
}

void Guild::addRank(uint32_t rankId, const std::string& rankName, uint8_t level)
{
	ranks.emplace_back(std::make_shared<GuildRank>(rankId, rankName, level));
}

void Guild::setPoints(uint32_t _points) {
	points = _points;
	IOGuild::setPoints(id, points);
}

void Guild::setLevel(uint32_t _level) {
	level = _level;
	IOGuild::setLevel(id, level);
}

Guild* IOGuild::loadGuild(uint32_t guildId)
{
	Database& db = Database::getInstance();
	std::ostringstream query;
	query << "SELECT `name`, `level`, `points` FROM `guilds` WHERE `id` = " << guildId;
	if (DBResult_ptr result = db.storeQuery(query.str())) {
		Guild* guild = new Guild(guildId, result->getString("name"));

		// Removendo a atribuição do saldo bancário
		// guild->setBankBalance(result->getNumber<uint64_t>("balance"));

		guild->setLevel(result->getNumber<uint32_t>("level"));
		guild->setPoints(result->getNumber<uint32_t>("points"));

		query.str(std::string());
		query << "SELECT `id`, `name`, `level` FROM `guild_ranks` WHERE `guild_id` = " << guildId;

		if ((result = db.storeQuery(query.str()))) {
			do {
				guild->addRank(result->getNumber<uint32_t>("id"), result->getString("name"), result->getNumber<uint16_t>("level"));
			} while (result->next());
		}
		return guild;
	}
	return nullptr;
}

uint32_t IOGuild::getGuildIdByName(const std::string& name)
{
	Database& db = Database::getInstance();

	DBResult_ptr result = db.storeQuery(fmt::format("SELECT `id` FROM `guilds` WHERE `name` = {:s}", db.escapeString(name)));
	if (!result) {
		return 0;
	}
	return result->getNumber<uint32_t>("id");
}


void IOGuild::setLevel(uint32_t guildId, uint32_t newlevel)
{
	Database& db = Database::getInstance();
	std::ostringstream query;
	query << "UPDATE `guilds` SET `level` = " << newlevel << " WHERE `id` = " << guildId;
	db.executeQuery(query.str());
}

void IOGuild::setPoints(uint32_t guildId, uint32_t newPoints)
{
	Database& db = Database::getInstance();
	std::ostringstream query;
	query << "UPDATE `guilds` SET `points` = " << newPoints << " WHERE `id` = " << guildId;
	db.executeQuery(query.str());
}
