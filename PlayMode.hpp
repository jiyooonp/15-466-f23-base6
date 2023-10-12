#include "Mode.hpp"

#include "Connection.hpp"
#include "Game.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode(Client &client);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking for local player:
	Player::Controls controls;

	//latest game state (from server):
	Game game;

	//last message from server:
	std::string server_message;

	//connection to server:
	Client &client;

	// draw lines for player_draw
	std::vector< glm::vec2 > player_draws;

	int level = 0;
	int score = 0;
	// word list
	std::vector<std::string> word_list = {
		"apple", "bicycle", "mountain", "guitar", "elephant", "computer", "butterfly", "umbrella", "penguin", "banana", "glasses", "pyramid", "mushroom", "kangaroo", "lighthouse", "robot", "spaghetti", "volcano", "pineapple", "helicopter", "octopus", "lipstick", "skyscraper", "cactus", "zeppelin", "yacht", "jigsaw", "koala", "necktie", "waterfall", "quill", "donut", "violin", "rhinoceros", "igloo", "puzzle", "flamingo", "xylophone", "chameleon", "accordion",
		"balloon", "grapes", "hedgehog", "island", "lamp", "telescope", "mailbox", "onion", "unicorn", "chocolate", "rocket", "squirrel", "turtle", "kiwi", "yo-yo", "zucchini", "vampire", "rainbow", "fountain"};
};
