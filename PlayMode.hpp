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

	int score = 0;
	// word list
	std::vector<std::string> word_list = {
		"apple", "banana", "tree", "book", "star",
		"circle", "square", "heart", "car", "sun",
		"moon", "cat", "dog", "boat", "flower",
		"hat", "door", "cloud", "fish", "bird",
		"bottle", "pen", "glasses", "shoe", "umbrella",
		"grape", "leaf", "pizza", "cake", "mountain",
		"ring", "chair", "table", "ball", "kite",
		"ice cream", "spoon", "fork", "toothbrush", "key",
		"house", "ladder", "rainbow", "pants", "shirt",
		"sock", "butterfly", "egg", "bell", "bridge",
		"light bulb", "mug", "pillow", "envelope", "flag",
		"drum", "pencil", "tooth", "hand", "smile"};
};
