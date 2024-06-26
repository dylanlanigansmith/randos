#include "gfx.h"
#include "font8x8_basic.h"

#include "../../kernel/stdlib.h"




vec2 last_draw = {0,0}; 

void update_last_draw(const vec2 pos){
    last_draw.x = pos.x;
    last_draw.y = pos.y;
   
}
void reset_last_draw() {  last_draw.x = GFX_TEXT_LEFT_MARGIN; last_draw.y =  GFX_TEXT_LINE_SPACING; }
/*
void __gfx_debug_v2(const vec2 dbg1, const vec2 dbg2){

    //this shows why yr gfx api has flaws
    vec2 ld = last_draw;
    gfx_clear_from_pos(clr_white, v2(0,100));
    const vec2 dbg_pos = {5, 150};
    gfx_print_at(itoa(dbg1.x, 10), dbg_pos);
    gfx_print_at(itoa(dbg1.y, 10), v2(dbg_pos.x + 5*FONT_W, dbg_pos.y));


    const vec2 dbg_pos2 = {5, 165};
    gfx_print_at(itoa(dbg2.x, 10), dbg_pos2);
    gfx_print_at(itoa(dbg2.y, 10), v2(dbg_pos2.x + 5*FONT_W, dbg_pos2.y));

    last_draw = ld;
}*/

gfx_state_t gfx_state;

void text_pos_to_pixel(const vec2 text, vec2* pos){
    pos->x = (text.x + FONT_W + GFX_TEXT_LEFT_MARGIN); 
    if(pos->x > (SCREEN_W - 1) ) pos->x = SCREEN_W - 1;
    pos->y =  (text.y + (2 * FONT_H) + GFX_TEXT_LINE_SPACING); 
    if(pos->y > (SCREEN_H - 1) ) pos->y = SCREEN_H - 1;

}

void gfx_clear( const color clr)
{
     for (int w = 0; w < SCREEN_W; w++){
        for(int h = 0; h < SCREEN_H; h++){
            set_pixel(v2(w,h), clr );
        }
   }
   // memset((void*)(FB_ADDR), clr, FB_SIZE);
    gfx_state.clear_color = clr;
    reset_last_draw();
}



void gfx_clear_text()
{
    if(last_draw.y < SCREEN_H - 1)
        last_draw.y += 2;
    
    if(last_draw.x < SCREEN_W - 10)
        last_draw.x += 10;
    
    vec2 pos = {0,0};
    text_pos_to_pixel(last_draw, &pos);
    
    
    color clr = gfx_state.clear_color;
    gfx_clear_to_pos(clr, pos); //so we really only need to clear current row / last two rows for performance
    
    reset_last_draw();
}
void gfx_clear_line(uint32_t y, uint32_t height)
{
    color clr = color_red; //gfx_state.clear_color;
    uintptr_t start_addr = (FB_ADDR) + (SCREEN_PITCH * y);
    uintptr_t end_addr = start_addr + SCREEN_PITCH * height;
    ASSERT(end_addr < FB_END_ADDR && start_addr >= FB_ADDR);

    if( (height % 2) > 0) height++;
    size_t num_ints = (SCREEN_PITCH / SCREEN_BPP) * height;
    //agh
    uint32_t* pixels = (uint32_t*)(start_addr);
    if(!memset_u32(pixels, clr.argb, num_ints)){
        KPANIC("gfx memset failed");
    }

}
void gfx_clear_to_pos(const color clr, const vec2 pos) // i hate this function we arent friends
{

#define CLEAR_FAST 
#ifndef CLEAR_FAST
 for(uint32_t y = 0; y < pos.y; ++y){
        for(uint32_t x = 0; x < SCREEN_W; ++x)
            set_pixel(v2(x,y), clr);
    } return;
#endif

  //  void* addr = (void*)(get_pixel(pos));
   // uintptr_t end_addr =  (void*)(get_pixel(v2(SCREEN_W - 1, SCREEN_H - 1)));
   // size_t num_pixels = ((uintptr_t)(addr) - FB_ADDR) / sizeof(uint32_t); //this is lazy

    
   // memset_u32((void*)(FB_ADDR), clr.argb,  num_pixels);

   //lol
   
   volatile uint32_t c = clr.argb;
   uint32_t y = pos.y;
   if(y % 2 != 0)
        y = (y != (SCREEN_H - 1) ) ? (y+1) : (SCREEN_H); // make it stupid so you remember it is likely bugged
    size_t num_ints = SCREEN_W * y;
  

     //bc keyboard throws interupts as we clearing and shit goes sideways 
   if(!memset_u32((uint32_t*)(FB_ADDR), c,  num_ints)){
        KPANIC("memset failure");
   }
   
  
        
}


void gfx_clear_from_pos(const color clr, const vec2 pos) //this is broken
{
    void* addr = (void*)(get_pixel(pos));
    uintptr_t end_addr =  (void*)(get_pixel(v2(SCREEN_W - 1, SCREEN_H - 1)));
    size_t num_pixels = ((uintptr_t)(addr) - FB_ADDR) / sizeof(uint32_t); //this is lazy

    debugf(_COM, "\n addr %lx end_addr %lx num %li x %i y %i \n", addr, end_addr, num_pixels, pos.x, pos.y);
    memset_u32((void*)(FB_ADDR), clr.argb,  num_pixels);
}
void gfx_init(const color clear_clr)
{
    debugf("==GFX INIT==/n");
    memset(&gfx_state, 0, sizeof(gfx_state_t));
    gfx_state.has_init=False; 
    *(uint32_t*)(&(gfx_state.draw_color)) = 0xff000000; 
    *(uint32_t*)(&(gfx_state.clear_color)) = 0xffffffff; 
    
    gfx_state.mode = 0;
    
    gfx_state.info.fb = FB_ADDR;
    gfx_state.info.w = SCREEN_W;
    gfx_state.info.h = SCREEN_H;
    gfx_state.info.p = SCREEN_PITCH;
    gfx_state.info.bpp = SCREEN_BPP;
    gfx_state.info.curmode = gfx_state.mode;

    gfx_clear(clear_clr);
    reset_last_draw();
    gfx_state.has_init = True;
    debugf("Graphics Init %i x %i x %i ready = %i \n", SCREEN_W, SCREEN_H, SCREEN_BPP * 8, gfx_state.has_init);
}

int gfx_has_init()
{
    return (gfx_state.has_init > 0);
}

void gfx_fill_rect(const vec2 pos, const vec2 size, const color clr)
{
    for(int x = 0; x < size.x; ++x)
        for(int y = 0; y < size.y; ++y)
            set_pixel(v2(pos.x + x, pos.y + y), clr);
}

void gfw_fill_rect_fast(const vec2 pos, const vec2 size, const color clr)
{

}

void make_new_line(vec2* pos){
    
    pos->x = GFX_TEXT_LEFT_MARGIN;
    pos->y = pos->y + (FONT_H + GFX_TEXT_LINE_SPACING);

    if (pos->y > SCREEN_H)
            pos->y = 0; // bad bad bad

   // update_last_draw(*pos);
    
}

void gfx_draw_str(const char *str, vec2 pos, const color clr) //TOUCHES LAST 
{
    for (int c = 0; str[c] != 0; ++c){
        if (str[c] == '\n' || str[c] == '\r'){
            make_new_line(&pos); continue;
        }
        
        gfx_draw_char(GET_FONT(str, c), pos, clr);
        pos.x += FONT_W;
        if (pos.x > (SCREEN_W - FONT_W + 2) ){
            make_new_line(&pos);
        }
       
    }
    update_last_draw(pos);
}



void gfx_draw_char(const char* font_char, const vec2 pos, const color clr) //TOUCHES LAST 
{
    int cx, cy;
    int mask[8] = {1,2,4,8,16,32,64,128};
    for(cy = 0; cy < FONT_H; ++cy){
        for(cx=0; cx < FONT_W; ++cx){
            if(font_char[cy] & mask[cx]) set_pixel(v2(pos.x + cx, pos.y + cy + FONT_H), clr);
        }
    }

    update_last_draw(pos); //messsy
    last_draw.x += FONT_W; //wtf
    

   
}



void gfx_print(const char *str) //TOUCHES LAST 
{
    gfx_print_at(str, last_draw);
}

void gfx_print_at(const char *str, const vec2 pos) //TOUCHES LAST 
{
    gfx_draw_str(str, pos, gfx_state.draw_color);
}

void gfx_printclr(const char *str, const color clr) //TOUCHES LAST 
{
    gfx_draw_str(str, last_draw, clr);
}





void gfx_putc(const char c) //print single char (ex: from keyboard scancode) //TOUCHES LAST 
{
    if(c == '\n' || c == '\r'){
         make_new_line(&last_draw); return;
    }
    if(c == '\b'){
        gfx_delc(); return;
    }
    if (last_draw.x > (SCREEN_W - FONT_W) ){
            make_new_line(&last_draw);
    }
    gfx_draw_char(GFX_FONT[c], last_draw, gfx_state.draw_color);

}

void __gfx_fill_char(const vec2 pos){
    const static vec2 char_size = {FONT_W, FONT_H};
    gfx_fill_rect(pos, char_size, gfx_state.clear_color); //wrapper for if clear color is ever an issue
}

void gfx_delc() //remove last char
{
    vec2 pos = last_draw;

    //todo: not have one billion versions of this all doing same shit but brokenly
    if(pos.x - FONT_W >= SCREEN_W && pos.y > 0){
        pos.x = (SCREEN_W - GFX_TEXT_LEFT_MARGIN) ; //not right
        pos.y -= (FONT_H + GFX_TEXT_LINE_SPACING);
    }    
    pos.x -= FONT_W;
    
    
    //gfx_draw_char(GFX_FONT[' '], pos, gfx_state.draw_color);
   
    __gfx_fill_char(v2(pos.x, pos.y + FONT_H));
    update_last_draw(pos);

    
}



/*
THESE DONT TOUCH CURSOR

*/
void gfx_draw_char_at(const char *font_char, const vec2 pos, const color clr)
{
    int cx, cy;
    int mask[8] = {1,2,4,8,16,32,64,128};
    for(cy = 0; cy < FONT_H; ++cy){
        for(cx=0; cx < FONT_W; ++cx){
            if(font_char[cy] & mask[cx]) set_pixel(v2(pos.x + cx, pos.y + cy + FONT_H), clr);
        }
    }
}
void gfx_print_pos(const char *str,  vec2 pos)
{
    //this is stopid
    for (int c = 0; str[c] != 0; ++c){
        if (str[c] == '\n' || str[c] == '\r'){
            make_new_line(&pos); continue;
        }
        
        gfx_draw_char_at(GET_FONT(str, c), pos, gfx_state.draw_color);
        pos.x += FONT_W;
        if (pos.x > (SCREEN_W - FONT_W + 2) ){
            make_new_line(&pos);
        }
       
    }
}