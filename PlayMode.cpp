#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <array>

PlayMode::PlayMode(Client &client_) : client(client_)
{
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_KEYDOWN)
	{
		if (evt.key.repeat)
		{
			// ignore repeats
		}
		else if (evt.key.keysym.sym == SDLK_LEFT)
		{
			controls.left.downs += 1;
			controls.left.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT)
		{
			controls.right.downs += 1;
			controls.right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP)
		{
			controls.up.downs += 1;
			controls.up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN)
		{
			controls.down.downs += 1;
			controls.down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_1){
			controls.guess.downs = 1;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_2){
			controls.guess.downs = 2;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_3){
			controls.guess.downs = 3;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_r)
		{
			// reset the game
			controls.guess.downs = 4;
			return true;
		}

		if (evt.key.keysym.sym == SDLK_SPACE)
		{
			controls.jump.downs += 1;
			controls.jump.pressed = true;
			return true;
		}
		
	}
	else if (evt.type == SDL_KEYUP)
	{
		if (evt.key.keysym.sym == SDLK_LEFT)
		{
			controls.left.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT)
		{
			controls.right.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP)
		{
			controls.up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN)
		{
			controls.down.pressed = false;
			return true;
		}

		if (evt.key.keysym.sym == SDLK_SPACE)
		{
			controls.jump.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{

	// queue data for sending to server:
	controls.send_controls_message(&client.connection);

	// reset button press counters:
	controls.left.downs = 0;
	controls.right.downs = 0;
	controls.up.downs = 0;
	controls.down.downs = 0;
	controls.jump.downs = 0;
	controls.guess.downs = 0;

	// send/receive data:
	client.poll([this](Connection *c, Connection::Event event)
				{
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			// std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush(); //DEBUG
			bool handled_message;
			try {
				do {
					handled_message = false;
					if (game.recv_state_message(c))
						handled_message = true;
					if (game.recv_game_state_message(c))
						handled_message = true;
				} while (handled_message);
			} catch (std::exception const &e) {
				std::cerr << "[" << c->socket << "] malformed message from server: " << e.what() << std::endl;
				//quit the game:
				throw e;
			}
		} },
	0.0);
	
	if (game.players.size() > 0)
	{
		if (score != int(game.game_state.game_score))
		{
			score = int(game.game_state.game_score);
			player_draws.clear();
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{


	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);


	// figure out view transform to center the arena:
	float aspect = float(drawable_size.x) / float(drawable_size.y);
	float scale = std::min(
		2.0f * aspect / (Game::ArenaMax[0].x - Game::ArenaMin[0].x + 2.0f * Game::PlayerRadius),
		2.0f / (Game::ArenaMax[0].y - Game::ArenaMin[0].y + 2.0f * Game::PlayerRadius));
	glm::vec2 offset = -0.5f * (Game::ArenaMax[0] + Game::ArenaMin[0]);

	glm::mat4 world_to_clip = glm::mat4(
		scale / aspect, 0.0f, 0.0f, offset.x,
		0.0f, scale, 0.0f, offset.y,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);


	{
		DrawLines lines(world_to_clip);

		// helper:
		auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H)
		{
			lines.draw_text(text,
							glm::vec3(at.x, at.y, 0.0),
							glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
							glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = (1.0f / scale) / drawable_size.y;
			lines.draw_text(text,
							glm::vec3(at.x + ofs, at.y + ofs, 0.0),
							glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
							glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};
		if (score < 0)
		{
			// draw the score to the screen
			std::string scoreString = "Final Score: " + std::to_string(int(game.game_state.game_score)) + ". Press \"r\" to restart!";
			draw_text(glm::vec2(-0.5f, 0.0f), scoreString, 0.09f);
			return;
		}

		// draws a box around the arena:
		lines.draw(glm::vec3(Game::ArenaMin[0].x, Game::ArenaMin[0].y, 0.0f), glm::vec3(Game::ArenaMax[0].x, Game::ArenaMin[0].y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		lines.draw(glm::vec3(Game::ArenaMin[0].x, Game::ArenaMax[0].y, 0.0f), glm::vec3(Game::ArenaMax[0].x, Game::ArenaMax[0].y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		lines.draw(glm::vec3(Game::ArenaMin[0].x, Game::ArenaMin[0].y, 0.0f), glm::vec3(Game::ArenaMin[0].x, Game::ArenaMax[0].y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		lines.draw(glm::vec3(Game::ArenaMax[0].x, Game::ArenaMin[0].y, 0.0f), glm::vec3(Game::ArenaMax[0].x, Game::ArenaMax[0].y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));


		for (auto const &player : game.players)
		{
			glm::u8vec4 col = glm::u8vec4(255, 0x00, 0x00, 0xff);

			// draw the score to the screen
			std::string scoreString = "Score: " + std::to_string(int(game.game_state.game_score));
			draw_text(glm::vec2(Game::ArenaMin[0].x - 0.6f, Game::ArenaMax[0].y - 0.5f * 4), scoreString, 0.09f);


			if (player.name == "Drawer")
			{
				// add all the traits of the player
				if (player.pressed_draw == 1)
					player_draws.push_back(player.position);
				for (auto p : player_draws)
				{
					lines.draw(
						glm::vec3(p, 0.0f),
						glm::vec3(p + glm::vec2(0.01f, 0.01f), 0.0f),
						col);
				}
				draw_text(player.position + glm::vec2(0.0f, -0.1f), "x", 0.09f);
			}
		}

		for (auto const &player : game.players)
		{

			if (player.name == "Drawer")
			{
				draw_text(player.position + glm::vec2(0.0f, -0.1f), "x", 0.09f);
				draw_text(glm::vec2(Game::ArenaMin[0].x - 0.6f, Game::ArenaMax[0].y - 1.0f), word_list[game.game_state.game_info[game.game_state.game_info.w]], 0.09f);
				break;
			}
			else if (player.name == "Guesser")
			{
				// draw the text for the guesser
				for (int i = 0; i < 3; i++)
				{
					std::string resultString = std::to_string(i + 1) + ". " + game.word_list[game.game_state.game_info[i]];
					draw_text(glm::vec2(Game::ArenaMin[0].x - 0.6f, Game::ArenaMax[0].y - 0.5f * (i + 1)), resultString, 0.09f);
				}
				break;
			}
		}
	}
	GL_ERRORS();
}