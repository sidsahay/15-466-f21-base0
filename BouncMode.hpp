#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

/*
 * BouncMode is a game mode that implements a single-player game of B.O.U.N.C.
 */

struct BouncMode : Mode {
	BouncMode();
	virtual ~BouncMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	float velocity_scale =4.0f;

	enum class PlayerState {
		AIR,
		GROUND
	};

	enum class BallState {
		CAN_HIT,
		FREE
	};

	struct Box {
		Box(const glm::vec2& position_, const glm::vec2& radius_) :
			position(position_), radius(radius_) {}
		glm::vec2 position;
		glm::vec2 radius;
	};
	std::vector<Box> boxes;
	std::vector<Box> shadow_boxes;
	std::vector<Box> shadow2_boxes;
	std::vector<Box> stars;

	Box moon_outline = Box(glm::vec2(-6.0f, 3.0f), glm::vec2(0.35f, 0.35f));
	Box moon_core = Box(glm::vec2(-6.0f, 3.0f), glm::vec2(0.3f, 0.3f));
	Box ground = Box(glm::vec2(0.0f, -7.0f), glm::vec2(10.0f, 1.0f));

	BallState ball_state = BallState::CAN_HIT;
	PlayerState player_state = PlayerState::AIR;
	glm::vec2 gravity = glm::vec2(0.0f, -13.0f);
	bool do_jump = false;
	bool do_bounce_jump = false;

	glm::vec2 court_radius = glm::vec2(10.0f, 5.0f);
	glm::vec2 player_radius = glm::vec2(0.2f, 0.2f);
	glm::vec2 ball_radius = glm::vec2(0.2f, 0.2f);

	glm::vec2 player = glm::vec2(-10.0f, 3.0f);
	glm::vec2 player_velocity = glm::vec2(0.0f, 0.0f);
	bool player_is_colliding = true;

	// spawn ball off screen
	glm::vec2 ball = glm::vec2(0.0f, 30.0f);
	glm::vec2 ball_velocity = glm::vec2(-1.0f, 0.0f);

	uint32_t lives = 5;

	// for animation oomph
	uint32_t exaggerated_frames = 0;

	//----- opengl assets / helpers ------

	//draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "BouncMode::Vertex should be packed");

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

};
