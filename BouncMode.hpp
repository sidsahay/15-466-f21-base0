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
	// game configuration
	const float velocity_scale = 4.0f;
	const float jump_velocity = 5.0f;
	const float bounce_velocity = 7.0f; // turn this down to <= 7.0f to make it extra frustrating!
	const glm::vec2 gravity = glm::vec2(0.0f, -13.0f);
	const glm::vec2 player_start = glm::vec2(-10.0f, 3.0f);

	// AIR: player is in air, can be affected by gravity
	// GROUND: player is on the ground, can't be affected by gravity
	enum class PlayerState {
		AIR,
		GROUND
	};

	// CAN_HIT: ball can hit player for a B.O.U.N.C. jump
	// FREE: ball will not collide with player
	enum class BallState {
		CAN_HIT,
		FREE
	};

	// convenience struct for map boxes and collision
	struct Box {
		Box(const glm::vec2& position_, const glm::vec2& radius_) :
			position(position_), radius(radius_) {}
		glm::vec2 position;
		glm::vec2 radius;
	};

	// randomly generated map geometry
	std::vector<Box> boxes;
	std::vector<Box> shadow_boxes;
	std::vector<Box> shadow2_boxes;
	std::vector<Box> stars;

	// terrible night sky
	const Box moon_outline = Box(glm::vec2(-6.0f, 3.0f), glm::vec2(0.35f, 0.35f));
	const Box moon_core = Box(glm::vec2(-6.0f, 3.0f), glm::vec2(0.3f, 0.3f));
	const Box ground = Box(glm::vec2(0.0f, -7.0f), glm::vec2(10.0f, 1.0f));

	// state variables for player and ball
	BallState ball_state = BallState::CAN_HIT;
	PlayerState player_state = PlayerState::AIR;

	// is there a pending jump or B.O.U.N.C. jump?
	bool do_jump = false;
	bool do_bounce_jump = false;

	// has the game ended?
	bool has_ended = false;

	// player and area parameters
	glm::vec2 court_radius = glm::vec2(10.0f, 5.0f);
	glm::vec2 player_radius = glm::vec2(0.2f, 0.2f);
	glm::vec2 ball_radius = glm::vec2(0.2f, 0.2f);

	glm::vec2 player = player_start;
	glm::vec2 player_velocity = glm::vec2(0.0f, 0.0f);

	// spawn ball off screen
	glm::vec2 ball = glm::vec2(0.0f, 30.0f);
	glm::vec2 ball_velocity = glm::vec2(-1.0f, 0.0f);

	uint32_t deaths = 0;

	// frame counter for animation oomph
	uint32_t exaggerated_frames = 0;

	// "font library"
	const glm::vec2 one_radius = glm::vec2(0.1f, 0.3f);
	const glm::vec2 zero_outer_radius = glm::vec2(0.2f, 0.3f);
	const glm::vec2 zero_inner_radius = glm::vec2(0.1f, 0.2f);

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
