#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)


float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 130, ball_dp_y, ball_half_size = 1; // no acceleration (makes it hard to predict, the wall will just float around)
// ball_dp_x = 100 (Original acceleration)

int player_1_score, player_2_score;

internal void
simulate_player(float* p, float* dp, float ddp, float dt)
{
	ddp -= *dp * 10.f;

	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;

	if (*p + player_half_size_y > arena_half_size_y)
	{
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0;
	}
	else if (*p - player_half_size_y < -arena_half_size_y)
	{
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0;
	}
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y + hs1y < p2y + hs2y);
}

enum Gamemode
{
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button; // stores which button (0 = single player, 1 = two player)
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt) {
	// clear_screen(0xff5500); the retarded way to draw borders
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);

	if (current_gamemode == GM_GAMEPLAY)
	{
		//float speed = 50.f; // units per second
		float player_1_ddp = 0.f;
		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp += 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
		}
		else {
			//if (ball_p_y > player_1_p + 2.f) player_1_ddp += 1300;
			//if (ball_p_y < player_1_p - 2.f) player_1_ddp -= 1300;
			player_1_ddp = (ball_p_y - player_1_p) * 100;
			if (player_1_ddp > 1300) player_1_ddp = 1300;
			if (player_1_ddp < -1300) player_1_ddp = -1300;
		}
		/*
				// AI (later improvments: predicting the ball movement (ex. if the ball is going really fast in y axis it also tries to move faster)
				// another thing is see if the balls will reflect off of a wall
				// another one is the enemy could try to hit the balls with the end of the pad so it will force the player to move instead of enemy always hitting the wall dead center like how it is now

		// the # (pound) symbol means that it will take effect before the compilation happens in the preprocessor
		#if 0
				if (is_down(BUTTON_UP)) player_1_ddp += 2000;
				if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
		#else

			//if (ball_p_y > player_1_p + 2.f) player_1_ddp += 1300; // if the ball is above player goes up (2000 acceleration is impossible) (the ball is farther away from the center of the enemy to move like 2.f (2 unit) that will fix the weird shaking)
			//if (ball_p_y < player_1_p - 2.f) player_1_ddp -= 1300; // if its below it will go down (decreasing the acceleration makes it easier)
			// move an amount based on the distance so if it's close it won't accelerate full force
				player_1_ddp = (ball_p_y - player_1_p) * 100;
				if (player_1_ddp > 1300) player_1_ddp = 1300; // limiter
				if (player_1_ddp < -1300) player_1_ddp = -1300; // limiter
		#endif
		*/
		float player_2_ddp = 0.f;
		if (is_down(BUTTON_W)) player_2_ddp += 2000;
		if (is_down(BUTTON_S)) player_2_ddp -= 2000;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);


		// Simulate Ball
		{
			/*
			// player 1 movement
			player_1_ddp -= player_1_dp * 10.f; // Friction

			player_1_p = player_1_p + player_1_dp * dt + player_1_ddp * dt * dt * .5f;
			player_1_dp = player_1_dp + player_1_ddp * dt;

			// 1D collision (only one axis is considered)
			if (player_1_p + player_half_size_y > arena_half_size_y) // top wall
			{
				player_1_p = arena_half_size_y - player_half_size_y;
				player_1_dp = 0; // stops the player completely
				//player_1_dp *= -2; inverting the velocity value so the player will bounce in the opposite direction

			}
			else if (player_1_p - player_half_size_y < -arena_half_size_y) // bottom wall (same thing but inverted)
			{
				player_1_p = -arena_half_size_y + player_half_size_y;
				player_1_dp = 0; // stops the player completely
			}

			// player 2 movement
			player_2_ddp -= player_2_dp * 10.f; // Friction

			player_2_p = player_2_p + player_2_dp * dt + player_2_ddp * dt * dt * .5f;
			player_2_dp = player_2_dp + player_2_ddp * dt;

			if (player_2_p + player_half_size_y > arena_half_size_y)
			{
				player_2_p = arena_half_size_y - player_half_size_y;
				player_2_dp = 0;
			}
			else if (player_2_p - player_half_size_y < -arena_half_size_y)
			{
				player_2_p = -arena_half_size_y + player_half_size_y;
				player_2_dp = 0;
			}
			*/

			// ball position equation with the first derivative
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y))
			{
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1; // moves inverted (x velocity) after collision 
				// ball_dp_y = player_1_dp * .75f; // changing the direction of the balls velocity based on the players own velocity 
				// ball_dp_y = (ball_p_y - player_1_p) * 2; // changing the direction of the balls velocity based on where it hit the player (hitting the very top the ball will go up and so on)
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y))
			{
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f;
			}

			/* AABB(Axis Aligned Bounding Box)
			colliding only  if penetrating positive x, negative x, positive y, negative y
			collision ball right side with the player left side
			AABB vs AABB collision (AABB collision test)

			if (ball_p_x + ball_half_size > 80 - player_half_size_x &&
				ball_p_x - ball_half_size < 80 + player_half_size_x &&
				ball_p_y + ball_half_size > player_1_p - player_half_size_y &&
				ball_p_y + ball_half_size < player_1_p + player_half_size_y) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1; // moves inverted (x velocity) after collision
				// ball_dp_y = player_1_dp * .75f; // changing the direction of the balls velocity based on the players own velocity
				// ball_dp_y = (ball_p_y - player_1_p) * 2; // changing the direction of the balls velocity based on where it hit the player (hitting the very top the ball will go up and so on)
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f;
			}
			else if (ball_p_x + ball_half_size > -80 - player_half_size_x &&
				ball_p_x - ball_half_size < -80 + player_half_size_x &&
				ball_p_y + ball_half_size > player_2_p - player_half_size_y &&
				ball_p_y + ball_half_size < player_2_p + player_half_size_y) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f;
			}*/

			if (ball_p_y + ball_half_size > arena_half_size_y) // ball collision with arena top
			{
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) // ball collision with arena bottom
			{
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}

			if (ball_p_x + ball_half_size > arena_half_size_x) // ball collision with arena right
			{
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) // ball collision with arena left
			{
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			}
		}


		// Score System

		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

		/*
		* adds a rectangle as point for each score

		float at_x = -80;
		for (int i = 0; i < player_1_score; i++)
		{
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x += 2.5f;
		}

		at_x = 80;
		for (int i = 0; i < player_2_score; i++)
		{
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x -= 2.5f;
		}
		*/

		// Rendering
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);

	}
	else {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button; // Red left is AI
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}

		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
		}
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
		}

		draw_text("PONG TUTORIAL", -73, 40, 2, 0xffffff);
		//draw_text("WATCH THE STEP BY STEP TUTORIAL ON", -73, 22, .75, 0xffffff);
		//draw_text("YOUTUBE.COM/DANZAIDAN", -73, 15, 1.22, 0xffffff);
	}
}