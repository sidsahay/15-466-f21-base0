#include "BouncMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

BouncMode::BouncMode() {

	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//set up the vertex array object to describe arrays of BouncMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 0 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 //offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte *)0 + 4*3 + 4*1 //offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1,1);
		std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

    {
        // construct the map - this could (should) be done via asset pipeline
        boxes.emplace_back(glm::vec2(-10.0f, -3.0f), glm::vec2(0.6f, 3.0f));
        boxes.emplace_back(glm::vec2(-6.0f, -3.0f), glm::vec2(0.6f, 3.0f));
        boxes.emplace_back(glm::vec2(-1.0f, -3.0f), glm::vec2(0.6f, 3.0f));
        boxes.emplace_back(glm::vec2(3.0f, -2.0f), glm::vec2(0.6f, 4.0f));
        boxes.emplace_back(glm::vec2(7.0f, -4.0f), glm::vec2(0.6f, 2.0f));
        boxes.emplace_back(glm::vec2(10.0f, -4.0f), glm::vec2(0.6f, 2.0f));

        // randomly generate background scenery for  a r t
        static std::mt19937 mt;
        // generate brown buildings
        for (uint32_t i = 0; i < 20; ++i) {
            float x = (mt() / float(mt.max())) * 20.0f - 10.0f;
            float w = (mt() / float(mt.max())) * 2.0f;
            float h = (mt() / float(mt.max())) * 3.0f;
            shadow_boxes.emplace_back(glm::vec2(x, -3.0f + (h - 3.0f)), glm::vec2(w, h));
        }

        // generate darker brown buildings
        for (uint32_t i = 0; i < 20; ++i) {
            float x = (mt() / float(mt.max())) * 20.0f - 10.0f;
            float w = (mt() / float(mt.max())) * 2.0f;
            float h = (mt() / float(mt.max())) * 5.0f;
            shadow2_boxes.emplace_back(glm::vec2(x, -3.0f + (h - 3.0f)), glm::vec2(w, h));
        }

        // generate stars - this should probably be done in a shader instead
        for (uint32_t i = 0; i < 50; ++i) {
            float x = (mt() / float(mt.max())) * 20.0f - 10.0f;
            float y = (mt() / float(mt.max())) * 10.0f - 5.0f;
            stars.emplace_back(glm::vec2(x, y), glm::vec2(0.03f, 0.03f));
        }
    }
}

BouncMode::~BouncMode() {

	//----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;
}

bool BouncMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    // on keydown, set player velocity, and if space, start jumping
	if (evt.type == SDL_KEYDOWN) {
        switch(evt.key.keysym.sym) {
            case SDLK_w:
                player_velocity.y = velocity_scale;
                break;
            case SDLK_s:
                player_velocity.y = -velocity_scale;
                break;
            case SDLK_a:
                player_velocity.x = -velocity_scale;
                break;
            case SDLK_d:
                player_velocity.x = velocity_scale;
                break;
            case SDLK_SPACE:
                if (player_state == PlayerState::GROUND) {
                    do_jump = true;
                    player_state = PlayerState::AIR;
                    // hyperextended frame time for animation oomph
                    exaggerated_frames = 10;
                }
                break;
        }
    }

    // on keyup, stop moving
    else if (evt.type == SDL_KEYUP) {
        switch(evt.key.keysym.sym) {
            case SDLK_w:
            case SDLK_s:
                player_velocity.y = 0.0f;
                break;
            case SDLK_a:
            case SDLK_d:
                player_velocity.x = 0.0f;
                break;
        }
    }

    // on mouse click, fire the B.O.U.N.C. projectile (ball)
    else if (evt.type == SDL_MOUSEBUTTONDOWN) {
        glm::vec2 clip_mouse = glm::vec2(
			(evt.button.x + 0.5f) / window_size.x * 2.0f - 1.0f,
			(evt.button.y + 0.5f) / window_size.y *-2.0f + 1.0f
		);

        // shoot the ball from the player
        ball = player;
        ball_velocity = glm::normalize((clip_to_court * glm::vec3(clip_mouse, 1.0f)) - ball) * 15.0f;
		// pre-shift the ball a bit so that the player can't cheese by firing upwards
		const float pre_shift = 0.03f;
		ball += pre_shift * ball_velocity;
        // ball starts being able to hit the player to cause a B.O.U.N.C. jump
        ball_state = BallState::CAN_HIT;
    }

	return false;
}

void BouncMode::update(float elapsed) {
	// check if the game was won
	// game is won if player is grounded and touching the right edge
	if (player_state == PlayerState::GROUND && (player.x + player_radius.x >= 10.0f)) {
		has_ended = true;
	}

	// do no updates if game has ended
	if (has_ended) return;

    // ----player state update----
    // only apply gravity when in the air
	if (player_state == PlayerState::AIR) {
        player_velocity += elapsed * gravity;
        player_velocity.y = std::max(player_velocity.y, -10.0f);
    }

    // apply instantaneous jerk if jumping or B.O.U.N.C. jumping
    if (do_jump) {
        player_velocity.y = jump_velocity;
    }
    if (do_bounce_jump) {
        player_velocity.y = bounce_velocity;
    }

    player += elapsed * player_velocity;
    do_jump = false;
    do_bounce_jump = false;

    // reset the player to start if the player falls off the map
    // This makes the game extra frustrating. Muhahahaha.
	if (player.y < -20.0f) {
        player = player_start;
		deaths++;
    }

	// clamp player to sides
	if (player.x >= 10.0f) {
		player.x = 10.0f;
	}
	else if (player.x <= -10.0f) {
		player.x = -10.0f;
	}

	//----- ball update -----

	ball += elapsed * ball_velocity;

	//---- collision handling ----

    // player vs box
    auto player_vs_box = [this](const Box& box) {
        // compute area of overlap
		glm::vec2 min = glm::max(player - player_radius, box.position - box.radius);
		glm::vec2 max = glm::min(player + player_radius, box.position + box.radius);

        if (min.x > max.x || min.y > max.y) {
            return false;
        }
        else {
            //if player is "above" box, set grounded and attach player to ground
            //don't use edges because of inaccuracies
            if ((box.position.y + box.radius.y) < player.y) {
                player_state = PlayerState::GROUND;
                player.y = box.position.y + box.radius.y + player_radius.y;
                player_velocity.y = 0;
            }
            // side collision: glue player to box side because they're dead anyway
            else {
                if (player.x < box.position.x) {
                    player.x = box.position.x - box.radius.x - player_radius.x;
                }
                else {
                    player.x = box.position.x + box.radius.x + player_radius.x;
                }
            }

            return true;
        }
    };
	{
		bool collided = false;
		for (const auto& box : boxes) {
			collided = collided || player_vs_box(box);
		}

		// if no collision happened and player was on the gound, player is in the air
		if (!collided && player_state == PlayerState::GROUND) {
			player_state = PlayerState::AIR;
		}
	}

	// compute ball-and-box collisions
	// side effect: reflects ball
    auto ball_vs_box = [this](const Box& box) {
	    //compute area of overlap:
		glm::vec2 min = glm::max(box.position - box.radius, ball - ball_radius);
		glm::vec2 max = glm::min(box.position + box.radius, ball + ball_radius);

		//if no overlap, no collision:
		if (min.x > max.x || min.y > max.y) {
            return false;
        }

		if (max.x - min.x > max.y - min.y) {
			//wider overlap in x => bounce in y direction:
			if (ball.y > box.position.y) {
				ball.y = box.position.y + box.radius.y + ball_radius.y;
				ball_velocity.y = std::abs(ball_velocity.y);
			} else {
				ball.y = box.position.y - box.radius.y - ball_radius.y;
				ball_velocity.y = -std::abs(ball_velocity.y);
			}
		} else {
			//wider overlap in y => bounce in x direction:
			if (ball.x > box.position.x) {
				ball.x = box.position.x + box.radius.x + ball_radius.x;
				ball_velocity.x = std::abs(ball_velocity.x);
			} else {
				ball.x = box.position.x - box.radius.x - ball_radius.x;
				ball_velocity.x = -std::abs(ball_velocity.x);
			}
		}
        return true;
    };

    for (const auto& box : boxes) {
        ball_vs_box(box);
    }

	// also collision check with ground (unlike player who falls to their doom)
    ball_vs_box(ground);

    // compute player-and-ball collision
	// side effect: can trigger B.O.U.N.C. jumps if this is the
	// first time that the ball is hitting the player
	//compute area of overlap:
    glm::vec2 min = glm::max(player - player_radius, ball - ball_radius);
    glm::vec2 max = glm::min(player + player_radius, ball + ball_radius);

    //if no overlap, no collision:
    if (min.x > max.x || min.y > max.y) {

    }
    else {
        if (ball_state == BallState::CAN_HIT) {
            do_bounce_jump = true;
            ball_state = BallState::FREE;
        }
    }
}

void BouncMode::draw(glm::uvec2 const &drawable_size) {
	//some nice colors from the course web page:
	#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	// made bg_color a tad darker for night feel
	const glm::u8vec4 bg_color = HEX_TO_U8VEC4(0x102538ff);
	const glm::u8vec4 fg_color = HEX_TO_U8VEC4(0xf2d2b6ff);
	// made this a bit darker for backgorund buildings
	const glm::u8vec4 shadow_color = HEX_TO_U8VEC4(0x6b3d32ff);
	// even darker variant of above
    const glm::u8vec4 shadow2_color = HEX_TO_U8VEC4(0x291713ff);
	// for rim of moon image
    const glm::u8vec4 moon_outline_color = HEX_TO_U8VEC4(0x347cbaff);
	// for moon and stars
    const glm::u8vec4 moon_core_color = HEX_TO_U8VEC4(0xffe7e3ff);
	// for player
	const glm::u8vec4 player_color = HEX_TO_U8VEC4(0xb3391bff);
	// for ball
	const glm::u8vec4 ball_color = HEX_TO_U8VEC4(0xff8161ff);
	#undef HEX_TO_U8VEC4

	//other useful drawing constants:
	const float wall_radius = 0.05f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	//---- compute vertices to draw ----

	//vertices will be accumulated into this list and then uploaded+drawn at the end of this function:
	std::vector< Vertex > vertices;

	//inline helper function for rectangle drawing:
	// extra param: "lean" displaces two vertices to make the rectangle deform to point left or right
	// also used to make the buildings slightly asymmetrical for extra edginess
	auto draw_rectangle = [&vertices](glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color, float lean) {
		//draw rectangle as two CCW-oriented triangles:
		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x + (lean > 0 ? lean : 0), center.y+radius.y + (lean > 0 ? lean : 0), 0.0f), color, glm::vec2(0.5f, 0.5f));

		vertices.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x+radius.x + (lean > 0 ? lean : 0), center.y+radius.y + (lean > 0 ? lean : 0), 0.0f), color, glm::vec2(0.5f, 0.5f));
		vertices.emplace_back(glm::vec3(center.x-radius.x + (lean < 0 ? lean : 0), center.y+radius.y + (lean < 0 ? -lean :0), 0.0f), color, glm::vec2(0.5f, 0.5f));
	};

	// our hero is too edgy to *cast* a shadow

	// draw night sky
	// this should probably be done faster in the shader
    for (const auto& star : stars) {
        draw_rectangle(star.position, star.radius, moon_core_color, 0);    
    }

    draw_rectangle(moon_outline.position, moon_outline.radius, moon_outline_color, 0);
    draw_rectangle(moon_core.position, moon_core.radius, moon_core_color, 0);

	// draw city back to front
    for (const auto& shadow_box : shadow2_boxes) {
        draw_rectangle(shadow_box.position, shadow_box.radius, shadow2_color, 0.1f);
    }
    
    for (const auto& shadow_box : shadow_boxes) {
        draw_rectangle(shadow_box.position, shadow_box.radius, shadow_color, 0.1f);
    }
    
	// draw map
    for (const auto& box : boxes) {
        draw_rectangle(box.position, box.radius, fg_color, 0.1f);
    }

	// draw ground for debug purposes
    //draw_rectangle(ground.position, ground.radius, fg_color, 0);

	{
		// check if player is leaning left or right based on direction of motion
		// lean away, as if being blow back by the wind
		float player_lean = 0.0f;
		if (player_velocity.x < 0) {
			player_lean = 0.05f;
		}
		else if (player_velocity.x > 0) {
			player_lean = -0.05f;
		}
		
		// hyperextended frames for that animation oomph when jumping
		// interpolate linearly back to normal using frame count
		// TODO: do this in a frame independent way
		if (exaggerated_frames) {
			float lean_factor = 1.0f - (exaggerated_frames * 0.4f) ;
			player_lean *= lean_factor;
			exaggerated_frames--;
		}

		// render player
		draw_rectangle(player, player_radius, player_color, player_lean);
	}

	// render B.O.U.N.C. projectile
	draw_rectangle(ball, ball_radius, ball_color, 0);

	// death count
	glm::vec2 deaths_radius = glm::vec2(0.05f, 0.1f);
	for (uint32_t i = 0; i < deaths; ++i) {
		draw_rectangle(glm::vec2( -court_radius.x + (2.0f + 3.0f * i) * deaths_radius.x, court_radius.y + 2.0f * wall_radius + 2.0f * deaths_radius.y), deaths_radius, fg_color, 0);
	}

	// if game has ended, show deaths in binary
	// because I didn't have time to do fonts
	if (has_ended) {
		std::vector<uint32_t> bits;
		uint32_t d = deaths;
		do {
			bits.push_back(d % 2);
			d /= 2;
		} while (d > 0);

		const float sep = 0.05f;
		glm::vec2 start = glm::vec2((2 * zero_outer_radius.x + sep) * bits.size()/-2.0f, 3.0f);
		for (auto it = bits.rbegin(); it != bits.rend(); ++it) {
			if (*it) {
				draw_rectangle(start, one_radius, fg_color, 0);
			}
			else {
				draw_rectangle(start, zero_outer_radius, fg_color, 0);
				draw_rectangle(start, zero_inner_radius, bg_color, 0);
			}
			start.x += 2 * zero_outer_radius.x + sep;
		}
	}

	//------ compute court-to-window transform ------

	//compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		-court_radius.x - 2.0f * wall_radius - padding,
		-court_radius.y - 2.0f * wall_radius - padding
	);
	glm::vec2 scene_max = glm::vec2(
		court_radius.x + 2.0f * wall_radius + padding,
		court_radius.y + 2.0f * wall_radius + 3.0f * deaths_radius.y + padding
	);

	//compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	//we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	//compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	//build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	//NOTE: glm matrices are specified in *Column-Major* order,
	// so each line above is specifying a *column* of the matrix(!)

	//also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	//---- actual drawing ----

	//clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	//use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	//bind the solid white texture to location zero so things will be drawn just with their colors:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	//run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	//unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);
	

	GL_ERRORS(); //PARANOIA: print errors just in case we did something wrong.

}
