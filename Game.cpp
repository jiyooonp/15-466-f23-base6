#include "Game.hpp"

#include "Connection.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>

#include <glm/gtx/norm.hpp>

void Player::Controls::send_controls_message(Connection *connection_) const {
	assert(connection_);
	auto &connection = *connection_;

	uint32_t size = 6;
	connection.send(Message::C2S_Controls);
	connection.send(uint8_t(size));
	connection.send(uint8_t(size >> 8));
	connection.send(uint8_t(size >> 16));
	connection.send(uint8_t(size >> 24));

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
	send_button(guess);
}

bool Player::Controls::recv_controls_message(Connection *connection_) {
	assert(connection_);
	auto &connection = *connection_;

	auto &recv_buffer = connection.recv_buffer;

	//expecting [type, size_low0, size_mid8, size_high8]:
	if (recv_buffer.size() < 5) return false;
	if (recv_buffer[0] != uint8_t(Message::C2S_Controls)) return false;
	uint32_t size = (uint32_t(recv_buffer[4]) << 24)
	              | (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	if (size != 6) throw std::runtime_error("Controls message with size " + std::to_string(size) + " != 6!");
	
	//expecting complete message:
	if (recv_buffer.size() < 5 + size) return false;

	auto recv_button = [](uint8_t byte, Button *button) {
		button->pressed = (byte & 0x80);
		uint32_t d = uint32_t(button->downs) + uint32_t(byte & 0x7f);
		if (d > 255) {
			std::cerr << "got a whole lot of downs" << std::endl;
			d = 255;
		}
		button->downs = uint8_t(d);
	};

	recv_button(recv_buffer[5+0], &left);
	recv_button(recv_buffer[5+1], &right);
	recv_button(recv_buffer[5+2], &up);
	recv_button(recv_buffer[5+3], &down);
	recv_button(recv_buffer[5 + 4], &jump);
	recv_button(recv_buffer[5 + 5], &guess);

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + 5 + size);

	return true;
}


//-----------------------------------------

Game::Game() : mt(0x15466666) {
}

Player *Game::spawn_player() {
	if (players.size() >= 2) throw std::runtime_error("This is a two player game!");

	// if first player 

	players.emplace_back();
	Player &player = players.back();



	do {
		player.color.r = mt() / float(mt.max());
		player.color.g = mt() / float(mt.max());
		player.color.b = mt() / float(mt.max());
	} while (player.color == glm::vec3(0.0f));
	player.color = glm::normalize(player.color);

	if (next_player_number % 2 == 0) // has to be drawer
	{
		player.name = "Drawer";
		// random point in the middle area of the arena:
		player.position = glm::vec2(0.0f, 0.0f);
		std::cout << "drawer generated" << std::endl;
	}
	else{
		player.name = "Guesser";
		player.position = glm::vec2(-1.2f, 0.0f);
		std::cout << "Guesser generated" << std::endl;
	}
	next_player_number ++;

	new_level();
	return &player;
}
void Game::new_level(){
	level += 1;
	// select words of interest
	word_candidate_indeces.clear();
	for (int i = 0; i < 3; i++)
	{
		int selected = mt() % word_list.size();
		if (std::find(word_candidate_indeces.begin(), word_candidate_indeces.end(), selected) == word_candidate_indeces.end())
			word_candidate_indeces.push_back(selected);
	}
	// std::cout << "word candidates: " << word_list[word_candidate_indeces[0]] << ", " << word_list[word_candidate_indeces[1]] << ", " << word_list[word_candidate_indeces[2]] << std::endl;
	target_word_index = mt() % 3;
	// std::cout << "target word: " << word_list[word_candidate_indeces[target_word_index]] << std::endl;

	// put the things in game_state
	game_state.game_info[0] = word_candidate_indeces[0];
	game_state.game_info[1] = word_candidate_indeces[1];
	game_state.game_info[2] = word_candidate_indeces[2];
	game_state.game_info[3] = target_word_index;
	game_state.game_score = score;
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
		if (p.controls.left.pressed) dir.x -= 1.0f;
		if (p.controls.right.pressed) dir.x += 1.0f;
		if (p.controls.down.pressed) dir.y -= 1.0f;
		if (p.controls.up.pressed) dir.y += 1.0f;

		if (p.controls.jump.pressed) {
			p.pressed_draw = 1;
			// std::cout << "draw pressed!"<<std::endl;
			// std::cout <<  "p.pressed_draw: " << p.pressed_draw << std::endl;
			}
		else
			p.pressed_draw = false;

		// if the gusser guessed something
		if (p.name == "Guesser" && p.controls.guess.downs >0){
			// std::cout << "Player guessed: " << word_list[word_candidate_indeces[p.controls.guess.downs]] << std::endl;
			if (p.controls.guess.downs == target_word_index + 1){
				// guesser guessed correctly
				// std::cout << "Guesser guessed correctly!" << std::endl;
				score += 1;
			}
			else{
				// guesser guessed incorrectly
				// std::cout << "Guesser guessed incorrectly!" << std::endl;
				score -= 1;
			}
			new_level();
		}

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
		p.position += p.velocity * elapsed;

		//reset 'downs' since controls have been handled:
		p.controls.left.downs = 0;
		p.controls.right.downs = 0;
		p.controls.up.downs = 0;
		p.controls.down.downs = 0;
		p.controls.jump.downs = 0;
		p.controls.guess.downs = 0;
	}

	//collision resolution:
	for (auto &p1 : players) {
		//player/arena collisions:
		int player_int = (p1.name == "Drawer") ? 0 : 1;

		if (p1.position.x < ArenaMin[player_int].x)
		{
			p1.position.x = ArenaMin[player_int].x;
			p1.velocity.x = std::abs(p1.velocity.x);
		}
		if (p1.position.x > ArenaMax[player_int].x)
		{
			p1.position.x = ArenaMax[player_int].x;
			p1.velocity.x =-std::abs(p1.velocity.x);
		}
		if (p1.position.y < ArenaMin[player_int].y)
		{
			p1.position.y = ArenaMin[player_int].y;
			p1.velocity.y = std::abs(p1.velocity.y);
		}
		if (p1.position.y > ArenaMax[player_int].y)
		{
			p1.position.y = ArenaMax[player_int].y;
			p1.velocity.y =-std::abs(p1.velocity.y);
		}
	}

}


void Game::send_state_message(Connection *connection_, Player *connection_player) const {
	int arg_len = 4;
	assert(connection_);
	auto &connection = *connection_;

	connection.send(Message::S2C_State);
	//will patch message size in later, for now placeholder bytes:
	for (int i=0; i<arg_len; ++i){
		connection.send(uint8_t(0));
	}
	size_t mark = connection.send_buffer.size(); //keep track of this position in the buffer


	//send player info helper:
	auto send_player = [&](Player const &player) {
		connection.send(player.position);
		connection.send(player.velocity);
		connection.send(player.color);
		connection.send(player.pressed_draw);

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

	for (int i = arg_len; i > 0; i--)
	{
		connection.send_buffer[mark - i] = uint8_t(size >> (8 * (arg_len - i)));
	}
}

bool Game::recv_state_message(Connection *connection_) {
	long unsigned int arg_len = 4 + 1;
	assert(connection_);
	auto &connection = *connection_;
	auto &recv_buffer = connection.recv_buffer;

	if (recv_buffer.size() < arg_len)
		return false;
	if (recv_buffer[0] != uint8_t(Message::S2C_State)) return false;
	uint32_t size = (uint32_t(recv_buffer[4]) << 24)
				  | (uint32_t(recv_buffer[3]) << 16)
	              | (uint32_t(recv_buffer[2]) << 8)
	              |  uint32_t(recv_buffer[1]);
	uint32_t at = 0;
	//expecting complete message:
	if (recv_buffer.size() < arg_len + size)
		return false;

	//copy bytes from buffer and advance position:
	auto read = [&](auto *val) {
		if (at + sizeof(*val) > size) {
			throw std::runtime_error("Ran out of bytes reading state message.");
		}
		std::memcpy(val, &recv_buffer[arg_len + at], sizeof(*val));
		at += sizeof(*val);
	};

	players.clear();
	uint8_t player_count;
	read(&player_count);
	for (uint8_t i = 0; i < player_count; ++i) {
		players.emplace_back();
		Player &player = players.back();
		read(&player.position);
		read(&player.velocity);
		read(&player.color);
		read(&player.pressed_draw);
		uint8_t name_len;
		read(&name_len);
		//n.b. would probably be more efficient to directly copy from recv_buffer, but I think this is clearer:
		player.name = "";
		for (uint8_t n = 0; n < name_len; ++n) {
			char c;
			read(&c);
			player.name += c;
		}
	}

	if (at != size) throw std::runtime_error("Trailing data in state message.");

	//delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + arg_len + size);

	return true;
}

void Game::send_game_state_message(Connection *connection_, GameState *connection_game_state) const
{
	int arg_len = 2;
	assert(connection_);
	auto &connection = *connection_;

	connection.send(Message::S2C_State);
	// will patch message size in later, for now placeholder bytes:
	for (int i = 0; i < arg_len; ++i)
	{
		connection.send(uint8_t(0));
	}
	size_t mark = connection.send_buffer.size(); // keep track of this position in the buffer

	// send player info helper:
	auto send_game_state = [&](GameState const &game_state)
	{
		connection.send(game_state.game_info);
		connection.send(game_state.game_score);
	};

	// player count:
	connection.send(uint8_t(sizeof(game_state)));
	if (connection_game_state)
		send_game_state(*connection_game_state);
	// if (&player == connection_player)
	// 	continue;
	// send_player(player);

	// compute the message size and patch into the message header:
	uint32_t size = uint32_t(connection.send_buffer.size() - mark);

	for (int i = arg_len; i > 0; i--)
	{
		connection.send_buffer[mark - i] = uint8_t(size >> (8 * (arg_len - i)));
	}
}

bool Game::recv_game_state_message(Connection *connection_)
{
	long unsigned int arg_len = 2 + 1;
	assert(connection_);
	auto &connection = *connection_;
	auto &recv_buffer = connection.recv_buffer;

	if (recv_buffer.size() < arg_len)
		return false;
	if (recv_buffer[0] != uint8_t(Message::S2C_State))
		return false;
	uint32_t size = uint32_t(recv_buffer[1]);
	uint32_t at = 0;
	// expecting complete message:
	if (recv_buffer.size() < arg_len + size)
		return false;

	// copy bytes from buffer and advance position:
	auto read = [&](auto *val)
	{
		if (at + sizeof(*val) > size)
		{
			throw std::runtime_error("Ran out of bytes reading state message.");
		}
		std::memcpy(val, &recv_buffer[arg_len + at], sizeof(*val));
		at += sizeof(*val);
	};

	uint8_t player_count;
	read(&player_count);

	read(&game_state.game_info);
	read(&game_state.game_score);

	if (at != size)
		throw std::runtime_error("Trailing data in state message.");

	// delete message from buffer:
	recv_buffer.erase(recv_buffer.begin(), recv_buffer.begin() + arg_len + size);

	return true;
}
