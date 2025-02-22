#include "Game.hpp"

#include "Connection.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

#include <glm/gtx/norm.hpp>

void Player::Controls::send_controls_message(Connection *connection_) const {
	assert(connection_);
	auto &connection = *connection_;
	// std::cout << "scm" << std::endl;

	uint32_t size = 5;
	connection.send(Message::C2S_Controls);
	connection.send(uint8_t(size));
	connection.send(uint8_t(size >> 8));
	connection.send(uint8_t(size >> 16));

	auto send_button = [&](Button const &b) {
		if (b.downs & 0x80) {
			std::cerr << "Wow, you are really good at pressing buttons!" << std::endl;
		}
		connection.send(uint8_t( (b.pressed ? 0x80 : 0x00) | (b.downs & 0x7f) ) );
	};

	send_button(left);
	send_button(right);
	send_button(up);
	send_button(down);
	send_button(jump);
}



void Player::Controls::send_clicks_message(Connection *connection_) const {
	// std::cout << "scm" << std::endl;
	assert(connection_);
	auto &connection = *connection_;

	uint32_t size = 5;
	connection.send(Message::C2S_Clicks);
	connection.send(uint8_t(size));
	connection.send(uint8_t(size >> 8));
	connection.send(uint8_t(size >> 16));

	auto send_click = [&](ClickGrid const &b) {
		if (b.grid_spot & 0x80) {
			std::cerr << "Wow, you are really good at pressing buttons!" << std::endl;
		}
		connection.send(uint8_t( (b.pressed ? 0x80 : 0x00) | (b.grid_spot & 0x7f) ) );
	};

	send_click(clickgrid);
	send_click(clickgrid);
	send_click(clickgrid);
	send_click(clickgrid);
	send_click(clickgrid);
}



bool Player::Controls::recv_controls_message(Connection *connection_) {
	assert(connection_);
	auto &connection = *connection_;
	

	auto &recv_buffer = connection.recv_buffer;

	//expecting [type, size_low0, size_mid8, size_high8]:
	if (recv_buffer.size() < 4) return false;
	if (recv_buffer[0] != uint8_t(Message::C2S_Controls)) return false;
	uint32_t size = (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	if (size != 5) throw std::runtime_error("Controls message with size " + std::to_string(size) + " != 5!");
	
	//expecting complete message:
	if (recv_buffer.size() < 4 + size) return false;

	auto recv_button = [](uint8_t byte, Button *button) {
		button->pressed = (byte & 0x80);
		uint32_t d = uint32_t(button->downs) + uint32_t(byte & 0x7f);
		if (d > 255) {
			std::cerr << "got a whole lot of downs" << std::endl;
			d = 255;
		}
		button->downs = uint8_t(d);
	};

	recv_button(recv_buffer[4+0], &left);
	recv_button(recv_buffer[4+1], &right);
	recv_button(recv_buffer[4+2], &up);
	recv_button(recv_buffer[4+3], &down);
	recv_button(recv_buffer[4+4], &jump);

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + size);

	return true;
}

bool Player::Controls::recv_clicks_message(Connection *connection_) {
	
	assert(connection_);
	auto &connection = *connection_;
	// std::cout << "rcm" << std::endl;
	auto &recv_buffer = connection.recv_buffer;

	//expecting [type, size_low0, size_mid8, size_high8]:
	if (recv_buffer.size() < 4) {
		// std::cout << "recv_buffer.size() = " << recv_buffer.size() << " < 4 " << std::endl;
		return false;
	}
	if (recv_buffer[0] != uint8_t(Message::C2S_Clicks)){
		// std::cout << " recv_buffer[0] != uint8_t(Message::C2S_Clicks " << std::endl;
		// std::cout << unsigned(recv_buffer[0]) << std::endl;
		// std::cout << unsigned(Message::C2S_Clicks) << std::endl;
		return false;
	} 
	uint32_t size = (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	if (size != 5) throw std::runtime_error("Controls message with size " + std::to_string(size) + " != 5!");
	
	//expecting complete message:
	if (recv_buffer.size() < 4 + size){
		// std::cout << "recv_buffer.size(): " << recv_buffer.size() << std::endl;
		// std::cout << "  4 + size " <<  4 + size << std::endl;
		// std::cout << "incomplete message" << std::endl;
		return false;
	}


	// std::cout << "rcm !" << std::endl;
	auto recv_button = [](uint8_t byte, ClickGrid *button) {
		// button->pressed = (byte & 0x80);
		// std::cout << "button->grid_spot: " << unsigned(button->grid_spot) << std::endl;
		byte = button->grid_spot;
		// std::cout << "button->grid_spot: " << unsigned(button->grid_spot) << std::endl;
		// uint32_t d = uint32_t(button->grid_spot) + uint32_t(byte & 0x7f);
		// if (d > 255) {
		// 	std::cerr << "got a whole lot of downs" << std::endl;
		// 	d = 255;
		// }
		
		// button->grid_spot = uint8_t(d);
		// std::cout << "button->grid_spot: " << unsigned(button->grid_spot) << std::endl;
		// myclicked = uint8_t(d);
	};

	recv_button(recv_buffer[4+0], &clickgrid);
	recv_button(recv_buffer[4+1], &clickgrid);
	recv_button(recv_buffer[4+2], &clickgrid);
	recv_button(recv_buffer[4+3], &clickgrid);
	recv_button(recv_buffer[4+4], &clickgrid);

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + size);
	// std::cout << "clicsks " << std::endl;

	return true;
}


//-----------------------------------------

Game::Game() : mt(0x15466666) {
}

Player *Game::spawn_player() {
	players.emplace_back();
	Player &player = players.back();

	//random point in the middle area of the arena:
	player.position.x = glm::mix(ArenaMin.x + 2.0f * PlayerRadius, ArenaMax.x - 2.0f * PlayerRadius, 0.4f + 0.2f * mt() / float(mt.max()));
	player.position.y = glm::mix(ArenaMin.y + 2.0f * PlayerRadius, ArenaMax.y - 2.0f * PlayerRadius, 0.4f + 0.2f * mt() / float(mt.max()));

	// do {
	// 	player.color.r = mt() / float(mt.max());
	// 	player.color.g = mt() / float(mt.max());
	// 	player.color.b = mt() / float(mt.max());
	// } while (player.color == glm::vec3(0.0f));
	// player.color = glm::normalize(player.color);

	player.color = glm::vec3(0.0f);
	player.name = "Player " + std::to_string(next_player_number++);

	return &player;
}

void Game::remove_player(Player *player) {
	bool found = false;
	for (auto pi = players.begin(); pi != players.end(); ++pi) {
		if (&*pi == player) {
			players.erase(pi);
			found = true;
			break;
		}
	}
	assert(found);
}

void Game::update(float elapsed) {
	//position/velocity update:

	



	for (auto &p : players) {
		glm::vec2 dir = glm::vec2(0.0f, 0.0f);
	
		
		if (dir == glm::vec2(0.0f)) {
			//no inputs: just drift to a stop
			float amt = 1.0f - std::pow(0.5f, elapsed / (PlayerAccelHalflife * 2.0f));
			p.velocity = glm::mix(p.velocity, glm::vec2(0.0f,0.0f), amt);
		} else {
			//inputs: tween velocity to target direction
			dir = glm::normalize(dir);

			float amt = 1.0f - std::pow(0.5f, elapsed / PlayerAccelHalflife);

			//accelerate along velocity (if not fast enough):
			float along = glm::dot(p.velocity, dir);
			if (along < PlayerSpeed) {
				along = glm::mix(along, PlayerSpeed, amt);
			}

			//damp perpendicular velocity:
			float perp = glm::dot(p.velocity, glm::vec2(-dir.y, dir.x));
			perp = glm::mix(perp, 0.0f, amt);

			p.velocity = dir * along + glm::vec2(-dir.y, dir.x) * perp;
		}
		// p.position += p.velocity * elapsed;

		//reset 'downs' since controls have been handled:
		p.controls.left.downs = 0;
		p.controls.right.downs = 0;
		p.controls.up.downs = 0;
		p.controls.down.downs = 0;
		p.controls.jump.downs = 0;
	}



}


void Game::send_state_message(Connection *connection_, Player *connection_player) const {
	assert(connection_);
	auto &connection = *connection_;


	// std::cout << "ssm: myclicked- = " << unsigned(connection_player->controls.clickgrid.grid_spot) << std::endl;
	connection.send(Message::S2C_State);
	//will patch message size in later, for now placeholder bytes:
	connection.send(uint8_t(0));
	connection.send(uint8_t(0));
	connection.send(uint8_t(0));
	size_t mark = connection.send_buffer.size(); //keep track of this position in the buffer


	//send player info helper:
	auto send_player = [&](Player const &player) {
		// player.position.x = 
		// std::cout << "connection_player->controls.clickgrid.grid_spot:: " << float(connection_player->controls.clickgrid.grid_spot) << std::endl;
		const glm::vec2 barb2 = glm::vec2(float(connection_player->controls.clickgrid.grid_spot), float(connection_player->controls.clickgrid.grid_spot));
		connection.send(barb2);
		// connection.send(player.position);
		// std::cout << "barb.y " << barb.y  << std::endl;
		// connection.send(connection_player->controls.clickgrid.grid_spot);
		connection.send(player.velocity);
		const glm::vec3 barb = glm::vec3(float(connection_player->controls.clickgrid.grid_spot), float(connection_player->controls.clickgrid.grid_spot), float(connection_player->controls.clickgrid.grid_spot));
		
		// std::cout << "barb : " << barb.x << " " << barb.y << " " << barb.z << std::endl;
		
		connection.send(barb);
	
		//NOTE: can't just 'send(name)' because player.name is not plain-old-data type.
		//effectively: truncates player name to 255 chars
		uint8_t len = uint8_t(std::min< size_t >(255, player.name.size()));
		connection.send(len);
		connection.send_buffer.insert(connection.send_buffer.end(), player.name.begin(), player.name.begin() + len);
	};

	//player count:
	connection.send(uint8_t(players.size()));
	if (connection_player) send_player(*connection_player);
	for (auto const &player : players) {
		if (&player == connection_player) continue;
		send_player(player);
	}

	//compute the message size and patch into the message header:
	uint32_t size = uint32_t(connection.send_buffer.size() - mark);
	connection.send_buffer[mark-3] = uint8_t(size);
	connection.send_buffer[mark-2] = uint8_t(size >> 8);
	connection.send_buffer[mark-1] = uint8_t(size >> 16);
}

bool Game::recv_state_message(Connection *connection_) {
	assert(connection_);
	auto &connection = *connection_;
	auto &recv_buffer = connection.recv_buffer;

	if (recv_buffer.size() < 4) return false;
	if (recv_buffer[0] != uint8_t(Message::S2C_State)) return false;
	uint32_t size = (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	uint32_t at = 0;
	//expecting complete message:
	if (recv_buffer.size() < 4 + size) return false;

	//copy bytes from buffer and advance position:
	auto read = [&](auto *val) {
		if (at + sizeof(*val) > size) {
			throw std::runtime_error("Ran out of bytes reading state message.");
		}
		std::memcpy(val, &recv_buffer[4 + at], sizeof(*val));
		// std::cout <<" recv_buffer[4 + at]: " <<  recv_buffer[4 + at] << std::endl;
		at += sizeof(*val);
	};

	players.clear();
	uint8_t player_count;
	read(&player_count);
	for (uint8_t i = 0; i < player_count; ++i) {
		players.emplace_back();
		Player &player = players.back();
		read(&player.position);
		// std::cout << "player.position: " << player.position.x << "  " << player.position.y << std::endl;
		read(&player.velocity);
		read(&player.color);
		// std::cout << "player.color: " << player.color.x << "  " << player.position.y << " " << player.color.z << std::endl;
		uint8_t name_len;
		read(&name_len);
		// std::cout << "player.position.y " << player.position.y  << std::endl;
		//n.b. would probably be more efficient to directly copy from recv_buffer, but I think this is clearer:
		player.name = "";
		for (uint8_t n = 0; n < name_len; ++n) {
			char c;
			read(&c);
			player.name += c;
		}
		// std::cout << "player.position: " << player.position << std::endl;
	}

	if (at != size) throw std::runtime_error("Trailing data in state message.");

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 4 + size);
	
 	return true;
}
