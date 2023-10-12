#pragma once

#include <glm/glm.hpp>

#include <string>
#include <list>
#include <random>
#include <array>

struct Connection;

//Game state, separate from rendering.

//Currently set up for a "client sends controls" / "server sends whole state" situation.

enum class Message : uint8_t {
	C2S_Controls = 1, //Greg!
	S2C_State = 's',
	//...
};

//used to represent a control input:
struct Button {
	uint8_t downs = 0; //times the button has been pressed
	bool pressed = false; //is the button pressed now
};

//state of one player in the game:
struct Player {
	//player inputs (sent from client):
	struct Controls {
		Button left, right, up, down, jump, guess;

		void send_controls_message(Connection *connection) const;

		//returns 'false' if no message or not a controls message,
		//returns 'true' if read a controls message,
		//throws on malformed controls message
		bool recv_controls_message(Connection *connection);
	} controls;

	//player state (sent from server):
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
	int pressed_draw = 0;
	glm::vec4 game_info = glm::vec4(0, 0, 0, 1);

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	std::string name = "";
};

// state of one player in the game:
struct GameState
{
	// player state (sent from server):
	glm::vec4 game_info = glm::vec4(0, 0, 0, -1);
	int game_score = 0;
};

struct Game {

	GameState game_state;

	std::list<Player> players;		//(using list so they can have stable addresses)
	Player *spawn_player(); //add player the end of the players list (may also, e.g., play some spawn anim)
	void remove_player(Player *); //remove player from game (may also, e.g., play some despawn anim)

	std::mt19937 mt; //used for spawning players
	uint32_t next_player_number = 0; //used for naming players

	Game();

	//state update function:
	void update(float elapsed);

	//constants:
	//the update rate on the server:
	inline static constexpr float Tick = 1.0f / 30.0f;

	//arena size:
	inline static constexpr std::array<glm::vec2, 2> ArenaMin = {
		glm::vec2(-1.0f, -1.0f), glm::vec2(-1.3f, -1.0f)};
	inline static constexpr std::array<glm::vec2, 2> ArenaMax = {
		glm::vec2(1.0f, 1.0f),
		glm::vec2(-1.1f, 1.0f)};

	//player constants:
	inline static constexpr float PlayerRadius = 0.06f;
	inline static constexpr float PlayerSpeed = 1.0f;
	inline static constexpr float PlayerAccelHalflife = 0.25f;
	

	//---- communication helpers ----

	//used by client:
	//set game state from data in connection buffer
	// (return true if data was read)
	bool recv_state_message(Connection *connection);

	//used by server:
	//send game state.
	//  Will move "connection_player" to the front of the front of the sent list.
	void send_state_message(Connection *connection, Player *connection_player = nullptr) const;

	bool recv_game_state_message(Connection *connection);
	void send_game_state_message(Connection *connection, GameState *connection_player = nullptr) const;
	void new_level();

	// for game logic
	int score = 0;
	int level = 1;
	
	int target_word_index = 0;
	std::vector<int> word_candidate_indeces = {0, 1, 2};

	// word list
	std::vector<std::string> word_list = {
		"apple", "bicycle", "mountain", "guitar", "elephant", "computer", "butterfly", "umbrella", "penguin", "banana", "glasses", "pyramid", "mushroom", "kangaroo", "lighthouse", "robot", "spaghetti", "volcano", "pineapple", "helicopter", "octopus", "lipstick", "skyscraper", "cactus", "zeppelin", "yacht", "jigsaw", "koala", "necktie", "waterfall", "quill", "donut", "violin", "rhinoceros", "igloo", "puzzle", "flamingo", "xylophone", "chameleon", "accordion",
		"balloon", "grapes", "hedgehog", "island", "lamp", "telescope", "mailbox", "onion", "unicorn", "chocolate", "rocket", "squirrel", "turtle", "kiwi", "yo-yo", "zucchini", "vampire", "rainbow", "fountain"};
};
