#include <bn_core.h>
#include <bn_display.h>
#include <bn_log.h>
#include <bn_keypad.h>
#include <bn_random.h>
#include <bn_rect.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_size.h>
#include <bn_string.h>
#include <bn_backdrop.h>
#include <bn_color.h>

#include "bn_sprite_items_dot.h"
#include "bn_sprite_items_square.h"
#include "common_fixed_8x16_font.h"


// Pixels / Frame player moves at
static constexpr bn::fixed SPEED = 3;
 static constexpr bn::fixed new_speed = SPEED*2;

// Width and height of the the player and treasure bounding boxes
static constexpr bn::size PLAYER_SIZE = {8, 8};
static constexpr bn::size TREASURE_SIZE = {8, 8};

// changing player and treasure position
static constexpr int player_x = 50;
static constexpr int player_y = -30;
static constexpr int treasure_x = -30;
static constexpr int treasure_y = 20;
// Full bounds of the screen
static constexpr int MIN_Y = -bn::display::height() / 2;
static constexpr int MAX_Y = bn::display::height() / 2;
static constexpr int MIN_X = -bn::display::width() / 2;
static constexpr int MAX_X = bn::display::width() / 2;

// Number of characters required to show the longest numer possible in an int (-2147483647)
static constexpr int MAX_SCORE_CHARS = 11;
static constexpr int MAX_SPEED_BOOST_CHARS = 11;
static constexpr int BOOST_FLASH = 11;



// Score location
static constexpr int SCORE_X = 70;
static constexpr int SCORE_Y = -70;

static constexpr int SPEEDCOUNT_X = -70;
static constexpr int SPEEDCOUNT_Y = 70;




int main()
{
    bn::core::init();
    // backdrop color
    bn::backdrop::set_color(bn::color(30, 15, 0));

    bn::random rng = bn::random();

    // Will hold the sprites for the score
    bn::vector<bn::sprite_ptr, MAX_SCORE_CHARS> score_sprites = {};
    bn::vector<bn::sprite_ptr, BOOST_FLASH> boost_word= {};
    bn::vector<bn::sprite_ptr, MAX_SPEED_BOOST_CHARS> speed_boosts = {};
    bn::sprite_text_generator text_generator(common::fixed_8x16_sprite_font);
   //bn::sprite_text_generator text_generator(common::fixed_32x64_sprite_font.h);
    

    int score = 0;
    int timer= 0;
    int speed_boost_count =3;
    bool speed_boost_activate=false;

    bn::sprite_ptr player = bn::sprite_items::square.create_sprite(player_x, player_y);
    bn::sprite_ptr treasure = bn::sprite_items::dot.create_sprite(treasure_x, treasure_y);

    while (true)
    {
        
        // Move player with d-pad
        

        if (bn::keypad::left_held())
        {
            if(speed_boost_activate){
                player.set_x(player.x() - new_speed);
            }
            else{
            player.set_x(player.x() - SPEED);}
        }


        if (bn::keypad::right_held())
        {
            if(speed_boost_activate){
                player.set_x(player.x() + new_speed);
            }
            else{
                player.set_x(player.x() + SPEED);}
        }


        if (bn::keypad::up_held())
        {

            if(speed_boost_activate){
                player.set_y(player.y() - new_speed);
            }
            else{
            player.set_y(player.y() - SPEED);}
        }


        if (bn::keypad::down_held())
        {
            if(speed_boost_activate){
                player.set_y(player.y() + new_speed);
            }
            else{
            player.set_y(player.y() + SPEED);}
            
        }

        // Restarting game///
        if(bn::keypad::start_pressed()){
            score=0;
            speed_boost_count=3;
            timer=0;
            speed_boost_activate = false;
            player.set_position(player_x,player_y);
            treasure.set_position(treasure_x,treasure_y);
            
        }

        // speed changed when a is pressed
        if(bn::keypad::a_pressed() && speed_boost_count>0 ){
        text_generator.generate(0, 0, "BOOST!", boost_word);
           timer=0;
             speed_boost_activate= true;
           speed_boost_count--;
        }
        //speed boost timer/
        if(speed_boost_activate){
            timer++;

            if(timer>=300){
                speed_boost_activate = false;
                boost_word.clear();
                timer=0;
            }
        }
        
        // if(bn::keypad::right_held()){
        //     if(speed_boost_activate){
        //         player.set_x(player.x() + SPEED);
        //     }
        //     else{
        //         player.set_x(player.x() + new_speed);
        //     }
        // }


        /*if(bn::keypad::a_held() && bn::keypad::left_held() ){
           player.set_x(player.x() - new_speed);
           speed_boost_count++;
        }
        if(bn::keypad::a_held() && bn::keypad::up_held() ){
           player.set_y(player.y() - new_speed);
           speed_boost_count++;
        }
        if(bn::keypad::a_held() && bn::keypad::down_held() ){
           player.set_y(player.y() + new_speed);
           speed_boost_count++;
        }*/
        
      
        // .x returns the horizontal position of the sprite
        // If the sprite moves to the left, it will come back to the right.
        if (player.x() < MIN_X)
        {
            // .set_x sets the horizontal position of the sprite
            // just using the max/min/ already available
            player.set_x(MAX_X);
        }

        // if sprite moves to the right, it will come back on the left
        if (player.x() > MAX_X)
        {
            player.set_x(MIN_X);
        }
        // if sprite goes up,will show up again at the bottom
        // .y returns the vertical position of the sprite
        if (player.y() < MIN_Y)
        {
            // .set_y will set the vertical position of the sprite
            player.set_y(MAX_Y);
        }
        // if sprite goes down it will come back from top of screen
        if(player.y()>MAX_Y){
            player.set_y(MIN_Y);
        }
        // The bounding boxes of the player and treasure, snapped to integer pixels
        bn::rect player_rect = bn::rect(player.x().round_integer(),
                                        player.y().round_integer(),
                                        PLAYER_SIZE.width(),
                                        PLAYER_SIZE.height());
        bn::rect treasure_rect = bn::rect(treasure.x().round_integer(),
                                          treasure.y().round_integer(),
                                          TREASURE_SIZE.width(),
                                          TREASURE_SIZE.height());

        // If the bounding boxes overlap, set the treasure to a new location an increase score
        if (player_rect.intersects(treasure_rect))
        {
            // Jump to any random point in the screen
            int new_x = rng.get_int(MIN_X, MAX_X);
            int new_y = rng.get_int(MIN_Y, MAX_Y);
            treasure.set_position(new_x, new_y);

            score++;
        }

        // Update score display
        bn::string<MAX_SCORE_CHARS> score_string = bn::to_string<MAX_SCORE_CHARS>(score);
        score_sprites.clear();
        text_generator.generate(SCORE_X, SCORE_Y,
                                score_string,
                                score_sprites);
                                                        

         bn::string<MAX_SPEED_BOOST_CHARS> speedboost_string = bn::to_string<MAX_SPEED_BOOST_CHARS>(speed_boost_count);
        speed_boosts.clear();
        text_generator.generate(SPEEDCOUNT_X, SPEEDCOUNT_Y,
                                speedboost_string,
                                speed_boosts);

       
        // Update RNG seed every frame so we don't get the same sequence of positions every time
        rng.update();

        bn::core::update();
    }
}