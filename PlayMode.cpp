#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <array>


PlayMode::PlayMode(Client &client_) : client(client_) {
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.repeat) {
			//ignore repeats
		} else if (evt.key.keysym.sym == SDLK_a) {
			std::cout << "a down " << std::endl;
			controls.left.downs += 1;
			controls.left.pressed = true;

			// controls.clickgrid.grid_spot = 1;
			// controls.clickgrid.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			controls.right.downs += 1;
			controls.right.pressed = true;

			// controls.clickgrid.grid_spot = 2;
			// controls.clickgrid.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			controls.up.downs += 1;
			controls.up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			controls.down.downs += 1;
			controls.down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			controls.jump.downs += 1;
			controls.jump.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			std::cout << "a up " << std::endl;
			controls.left.pressed = false;
			// controls.clickgrid.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			controls.right.pressed = false;
			// controls.clickgrid.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			controls.up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			controls.down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			controls.jump.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		float cell_width = (1000 - 280) / 3.0f;
		float cell_height = (720 - 0) / 3.0f;

		int col = int((mouse_x - 280) / cell_width);
		int row = int((720 - mouse_y) / cell_height);

		if ((0 <= col) && (col < 3) && (0 <= row) && (row < 3)){
		// if ((0 <= col < 3) && (0 <= row < 3)){
			std::cout << "grid square " << (row * 3) + (col + 1) << std::endl;


			if(grid_status[((row * 3) + (col + 1)) - 1] == 0){
				if (player_id == 1){
					grid_status[((row * 3) + (col + 1)) - 1] = 1;
				}
				controls.clickgrid.grid_spot = ((row * 3) + (col + 1)) - 1;
				controls.clickgrid.pressed = true;
					
			}

		}else{
			std::cout << "outside" << std::endl;
		}
		
	}

	return false;
}

void PlayMode::update(float elapsed) {

	auto check_win = [this](int ind1, int ind2, int ind3) {
		if ((grid_status[ind1] == grid_status[ind2] == grid_status[ind3]) && (grid_status[ind1] != 0)){
			winner_id = grid_status[ind1];
		}
	};

	check_win(1,4,7);
	check_win(0,1,2);
	check_win(3,4,5);
	check_win(2,5,8);
	check_win(2,4,6);
	check_win(0,4,8);
	check_win(6,7,8);
	if (winner_id != 0){
		std::cout << "winner is player " << winner_id << std::endl;
	}

	// if (check_win(1,4,7)){
	// 	std::cout << "winner is player " << 
	// }

	//queue data for sending to server:
	controls.send_controls_message(&client.connection);
	
	controls.send_clicks_message(&client.connection);

	//reset button press counters:
	controls.left.downs = 0;
	// controls.clickgrid.presse
	controls.right.downs = 0;
	controls.up.downs = 0;
	controls.down.downs = 0;
	controls.jump.downs = 0;

	//send/receive data:
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush(); //DEBUG
			bool handled_message;
			try {
				do {
					handled_message = false;
					if (game.recv_state_message(c)) handled_message = true;
				} while (handled_message);
			} catch (std::exception const &e) {
				std::cerr << "[" << c->socket << "] malformed message from server: " << e.what() << std::endl;
				//quit the game:
				throw e;
			}
		}
	}, 0.0);
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {

	// static std::array< glm::vec2, 16 > const circle = [](){
	// 	std::array< glm::vec2, 16 > ret;
	// 	for (uint32_t a = 0; a < ret.size(); ++a) {
	// 		float ang = a / float(ret.size()) * 2.0f * float(M_PI);
	// 		ret[a] = glm::vec2(std::cos(ang), std::sin(ang));
	// 	}
	// 	return ret;
	// }();

	// glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	// glClear(GL_COLOR_BUFFER_BIT);
	// glDisable(GL_DEPTH_TEST);
	
	// //figure out view transform to center the arena:
	// window_size = glm::uvec2(w, h);
	// SDL_GL_GetDrawableSize(window, &w, &h);
	// std::cout << w << " " << h << std::endl;
    float aspect = float(drawable_size.x) / float(drawable_size.y);
	// float aspect = float(1280) / float(720);

	// float aspect = float(window_size.x) / float(window_size.y);
	// float scale = std::min(
	// 	2.0f * aspect / (Game::ArenaMax.x - Game::ArenaMin.x + 2.0f * Game::PlayerRadius),
	// 	2.0f / (Game::ArenaMax.y - Game::ArenaMin.y + 2.0f * Game::PlayerRadius)
	// );
	// glm::vec2 offset = -0.5f * (Game::ArenaMax + Game::ArenaMin);

	// glm::mat4 world_to_clip = glm::mat4(
	// 	scale / aspect, 0.0f, 0.0f, offset.x,
	// 	0.0f, scale, 0.0f, offset.y,
	// 	0.0f, 0.0f, 1.0f, 0.0f,
	// 	0.0f, 0.0f, 0.0f, 1.0f
	// );
	glm::mat4 world_to_clip = glm::mat4(
		1.0f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	{
		DrawLines lines(world_to_clip);

		//helper:
		// auto draw_text = [&](glm::vec2 const &at, std::string const &text, float H) {
		// 	lines.draw_text(text,
		// 		glm::vec3(at.x, at.y, 0.0),
		// 		glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 		glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		// 	float ofs = (1.0f) / drawable_size.y;
		// 	lines.draw_text(text,
		// 		glm::vec3(at.x + ofs, at.y + ofs, 0.0),
		// 		glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
		// 		glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		// };
		auto to_ld = [](int coord_x, int coord_y){

			// float ws_x_real = ((ws_x+1.0f)/2.0f)*1280;
			// float ws_y_real = 720 - (((ws_y+1.0f)/2.0f)*720);
			// float coord_x_new = ((coord_x/1280.0f)*2.0f)-1.0f;
			// float coord_y_new = 

			float coord_x_new = ((float(coord_x)/1280.0f)*2.0)-1.0f;
			float coord_y_new = ((float(coord_y)/720.0f)*2.0)-1.0f;
			// std::cout << "coord_x_new: " << coord_x_new << std::endl;
			// std::cout << "coord_y_new: " << coord_y_new << std::endl;
			return glm::vec3(
				coord_x_new,
				coord_y_new,
				0.0f
			);
		};



		// float bl_x = -0.5;
		// float bl_y = -0.8;
		// float tr_x = 0.5;
		// float tr_y = 0.2;

		float bl_x = 0;
		float bl_y = 0;
		float tr_x = 1280;
		float tr_y = 720;
		// // n_sq = num squares per dim
		int n_sq = 3;

		glm::u8vec4 grid_color =  glm::u8vec4(0xff, 0x00, 0xff, 0xff);
		// glm::u8vec4 test_color =  glm::u8vec4(0xff, 0xff, 0xff, 0xff);



		

		// lines.draw(glm::vec3(bl_x, bl_y, 0.0f), glm::vec3(bl_x, tr_y, 0.0f), grid_color);
		// lines.draw(glm::vec3(bl_x, tr_y, 0.0f),  glm::vec3(tr_x, tr_y, 0.0f), grid_color);
		// lines.draw(glm::vec3(tr_x, tr_y, 0.0f), glm::vec3(tr_x, bl_y, 0.0f), grid_color);
		// lines.draw(glm::vec3(tr_x, bl_y, 0.0f), glm::vec3(bl_x, bl_y, 0.0f), grid_color);
		
		// lines.draw(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.5f, 0.5f, 0.0f), grid_color);
		// lines.draw(glm::vec3(0.9f, 0.9f, 0.0f), glm::vec3(-0.9f, 0.9f, 0.0f), grid_color);
		// lines.draw(glm::vec3(-0.9f, 0.9f, 0.0f), glm::vec3(-0.9f, -0.9f, 0.0f), grid_color);
		
		for (int i=0; i<n_sq+1; i++){
			lines.draw( 
						to_ld(
							((tr_x-bl_x)/n_sq)*(i) + bl_x, 
							bl_y
						), 
						to_ld(
							((tr_x-bl_x)/n_sq)*(i) + bl_x, 
							tr_y
						),
						grid_color
			);
			lines.draw( 
						to_ld(
							bl_x,
							((tr_y - bl_y)/n_sq)*i + bl_y
						),
						to_ld(
							tr_x,
							((tr_y - bl_y)/n_sq)*i + bl_y
						),
						grid_color
			);
		}

		for (int i=0; i<9; i++){
			std::cout << "gridsat" << i << ": " << grid_status[i] << std::endl;
			if (grid_status[i] == 1){
				float my_x = 0.0f;
				float my_y = 0.0f;
				if (i==6 || i==7 || i==8){
					my_y = 0.66;
				}
				else if (i==0 || i==1 || i==2){
					my_y = -0.66;
				}

				if (i==0 || i==3 || i==6){
					my_x = -0.66;
				}
				else if (i==2 || i==5 || i==8){
					my_x = 0.66;
				}
				lines.draw(glm::vec3(my_x - 0.1, my_y - 0.1, 0.0f), 
						glm::vec3(my_x + 0.1, my_y + 0.1, 0.0f),
						grid_color);

				lines.draw(glm::vec3(my_x + 0.1, my_y - 0.1, 0.0f), 
						glm::vec3(my_x - 0.1, my_y + 0.1, 0.0f),
						grid_color);

				// lines.draw(glm::vec3(my_x - 0.2, my_y + 0.2, 0.0f), 
				// 		glm::vec3(my_x - 0.2, my_y + 0.2, 0.0f),
				// 		grid_color);
			}
			// if ((grid_status[i] == 7) || (grid_status[i] == 8) || (grid_status[i] == 9)){
				
			// }
		}

		// int mouse_x, mouse_y;
		// SDL_GetMouseState(&mouse_x, &mouse_y);
		// float coord_x_new = ((float(mouse_x)/1280.0f)*2.0)-1.0f;
		// float coord_y_new = ((float(mouse_y)/720.0f)*2.0)-1.0f;
		// std::cout << "coord_x_new: " << mouse_x << std::endl;
		// std::cout << "coord_y_new: " << mouse_y << std::endl;



		// std::cout << "(" << mouse_x << ", " << mouse_y << ")" << std::endl;
		// int mouse_x, mouse_y;
		// SDL_GetMouseState(&mouse_x, &mouse_y);
		// float cell_width = (1000 - 280) / 3.0f;
		// float cell_height = (720 - 0) / 3.0f;

		// int col = int((mouse_x - 280) / cell_width);
		// int row = int((720 - mouse_y) / cell_height);

		// if ((0 <= col) && (col < 3) && (0 <= row) && (row < 3)){
		// // if ((0 <= col < 3) && (0 <= row < 3)){
		// 	// std::cout << "grid square " << (row * 3) + (col + 1) << std::endl;
		// }else{
		// 	// std::cout << "outside" << std::endl;
		// }
			
			

		// glm::vec3 bb  = convert_to_linesdraw(glm::vec2(5,3));
		// std::cout << bb.x << std::endl;
		// for (int i=0; i<num_squares_per_dim; i++){

		// }

		// lines.draw(glm::vec3(Game::ArenaMin.x, Game::ArenaMin.y, 0.0f), glm::vec,3(Game::ArenaMax.x, Game::ArenaMin.y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		// lines.draw(glm::vec3(Game::ArenaMin.x, Game::ArenaMax.y, 0.0f), glm::vec3(Game::ArenaMax.x, Game::ArenaMax.y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		// lines.draw(glm::vec3(Game::ArenaMin.x, Game::ArenaMin.y, 0.0f), glm::vec3(Game::ArenaMin.x, Game::ArenaMax.y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
		// lines.draw(glm::vec3(Game::ArenaMax.x, Game::ArenaMin.y, 0.0f), glm::vec3(Game::ArenaMax.x, Game::ArenaMax.y, 0.0f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));

		
	}
	GL_ERRORS();
}
