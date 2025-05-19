#include "base.h"
#include "thefont.h"
#include "sprite.h"
#include "map_1.h"

void delay(ushort after_ticks)
{
    if ((FCON & 2) != 0)
        FCON &= 0xfd;
    __DI();
    Timer0Interval = after_ticks;
    Timer0Counter = 0;
    Timer0Control = 0x0101;
    InterruptPending_W0 = 0;
    StopAcceptor = 0x50;
    StopAcceptor = 0xa0;
    StopControl = 2;
    __asm("nop");
    __asm("nop");
    __EI();
}

byte coin()
{
	//val(0x9000) ^= 1;
	return val(0xF00D) & 1;
	//return (~((val(0xF023) & 1) - (val(0xF00D)&1)) + val(0xf022) + val(0xf023))&1;
}

void draw_buf()
{
	word i;
	for (i=0;i<2048;i++)
	{
		val(0xf037) = 0;
		val(0xf800+i) = val(0x9000+i);
		val(0xf037) = 4;
		val(0xf800+i) = val(0x9000+i+0x800);
	}
}


const byte error[] = "BRK TRIGGERED";
const byte message[] = "OUT OF BOUNDS";

void custom_break()
{
	while(1)
	{

	}

}

void draw_map()
{
	word i = 0;
	word j = 0;
	for (i=0;i<2016;i++)
	{
		if ((i & 0x1F) == 0x18) i += 0x08;
		deref(0xf037) = 0;
		deref(0xf820 + i) = lightm[j];
		deref(0xf820 + i) |= deref(0x9000 + i);
		deref(0xf037) = 4;
		deref(0xf820 + i) = darkm[j];
		deref(0xf820 + i) |= deref(0x9800 + i);
		j++;
	}
}

void set_pixel(word x, word y,byte color) {
	word addr = (y<<5) + (x >> 3) + 0xF800;
	byte ty = 0x80>>(x & 7);
	val(0xF037) = 0;
	switch(color)
	{
		case 0:
			val(addr) &= ~ty;
			val(0xF037) = 4;
			val(addr) &= ~ty;
			break;
		case 1:
			val(addr) |= ty;
			val(0xF037) = 4;
			val(addr) &= ~ty;
			break;
		case 2:
			val(addr) &= ~ty;
			val(0xF037) = 4;
			val(addr) |= ty;
			break;
		case 3:
			val(addr) |= ty;
			val(0xF037) = 4;
			val(addr) |= ty;
			break;
	}
}


void plot_line (int x0, int y0, int x1, int y1, byte color)
{
  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  for (;;){
    set_pixel (x0,y0,color);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void drawbitmap(const byte* ptr, word offset, byte width, byte height, int color)
{
	word x;
	word y;
	word i;
	word j;
	word tmp;
	byte writeTwice;
	byte f037;

	i = 0;
	j = offset;

	tmp = offset & 0x1F;
	if (tmp >= 0x18)
	{
		j += 0x08;
	}

	writeTwice = (color == 2);
	f037 = (color == 1 || color == 2) ? 4 : 0;

	for (y = 0; y < height; ++y)
	{
		for (x = 0; x < width; ++x)
		{
			if (writeTwice)
			{
				deref(0xF037) = 0;
				deref(0xF800 + j) = ptr[i];
				deref(0xF037) = 4;
			}

			deref(0xF037) = f037;
			deref(0xF800 + j) = ptr[i];

			++j;
			if ((j & 0x1F) == 0x18) j += 0x08;
			++i;
		}

		j += (32 - width);
		if ((j & 0x1F) == 0x18) j += 0x08;
	}
}

word print(const byte* str, byte x, byte y, byte color)
{
	const byte* what;
	word curoffset;
	word count;

	what = str;
	count = 0;

	curoffset = (word)x + (((word)y) << 9);

	while (*what)
	{
		drawbitmap(image_raw + (((word)(*what)) << 4), curoffset, 1, 16, color);
		++curoffset;
		++what;
		++count;
	}
	return count;
}

void WordPrint(word str, byte x, byte y, byte color)
{
    char buff[5];
    byte i;
    for (i = 0; i < 4; i++)
    {
        byte nibble = (str >> ((3 - i) * 4)) & 0xF;
        if (nibble < 10)
            buff[i] = '0' + nibble;
        else
            buff[i] = 'A' + (nibble - 10);
    }
    buff[4] = '\0';
    print(buff, x, y, color);
}

void CharPrint(byte str, byte x, byte y, byte color)
{
    char buff[3];
    byte hi = (str >> 4) & 0xF;
    byte lo = str & 0xF;
    buff[0] = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
    buff[1] = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
    buff[2] = '\0';
    print(buff, x, y, color);
}



enum BUTTON
{
	B_0 = 0xb,
	B_1 = 0x3f,
	B_2 = 0x37,
	B_3 = 0x2f,
	B_4 = 0x3e,
	B_5 = 0x36,
	B_6 = 0x2e,
	B_7 = 0x3d,
	B_8 = 0x35,
	B_9 = 0x2d,

	B_A = 0x3c,
	B_B = 0x34,
	B_C = 0x2c,
	B_D = 0x24,
	B_E = 0x1c,
	B_F = 0x14,

	B_G = 0x3d,
	B_H = 0x35,
	B_I = 0x2d,
	B_J = 0x25,
	B_K = 0x1d,

	B_L = 0x3e,
	B_M = 0x36,
	B_N = 0x2e,
	B_O = 0x26,
	B_P = 0x1e,

	B_Q = 0x3f,
	B_R = 0x37,
	B_S = 0x2f,
	B_T = 0x27,
	B_U = 0x1f,

	B_V = 0xb,
	B_W = 0xc,
	B_X = 0xd,
	B_Y = 0xe,
	B_Z = 0xf,



	BUTTON_COUNT = 0x40
};

enum SPECIAL_CHARS
{
	SP_EXE = 0x0f,
	SP_TAB = '\t',
	SP_SPACE = ' ',
	SP_BACKSPACE = '\b',

	SP_HOME = 0x80,
	SP_END = 0x1A,
	SP_YES = 0x3A,
	SP_NO = 0x12,
	SP_OK = 0x21,
	SP_UP = 0x20,
	SP_DOWN = 0x22,
	SP_LEFT = 0x29,
	SP_RIGHT = 0x19,
	SP_PLUS = 0x10,
	SP_MINUS = 0x11,
	SP_ALT = 0x31,
	SP_ABC = 0x30,
	SP_ESC = 0x39,
	SP_SELECTLEFT,
	SP_SELECTRIGHT,
	SP_PASTE,
	SP_COPY,

	SPECIAL_MAX
};
const byte button_to_char[64] = {
	  0,        0,       0,    0,    0,    0,   0,    0,
	  0,        0,       0,  '0',  ' ',  ',', '.', '\n',
SP_PLUS, SP_MINUS,   SP_NO,  ']',  ')',    0,   0,    0,
	  0, SP_RIGHT,  SP_END,  '[',  '(', '\b', '#',  '%',
  SP_UP,    SP_OK, SP_DOWN, '\'',  '/',  '!', '@',  '$',
	  0,  SP_LEFT, SP_HOME,  ';', '\\',  '9', '6',  '3',
 SP_ABC,   SP_ALT,     '*',  '-',  '=',  '8', '5',  '2',
	  0,   SP_ESC,  SP_YES, '\t',  '`',  '7', '4',  '1',
};

const byte button_to_char_abc[64] = {
	0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0, 'V', 'W', 'X', 'Y', 'Z',
	0,   0,   0,   0, 'F',   0,   0,   0,
	0,   0,   0,   0, 'E', 'K', 'P', 'U',
	0,   0,   0,   0, 'D', 'J', 'O', 'T',
	0,   0,   0,   0, 'C', 'I', 'N', 'S',
	0,   0,   0,   0, 'B', 'H', 'M', 'R',
	0,   0,   0,   0, 'A', 'G', 'L', 'Q',
};

const byte button_to_char_alt[64] = {
	0,              0,        0,   0,   0,   0,   0,   0,
	0,              0,        0,   0,   0, '<', '>',   0,
	0,              0, SP_PASTE, '}', '^',   0,   0,   0,
	0, SP_SELECTRIGHT,        0, '{', '*',   0,   0,   0,
	0,              0,        0, '"', '?',   0,   0,   0,
	0,  SP_SELECTLEFT,        0, ':', '|',   0,   0,   0,
	0,              0,      '&', '_', '+',   0,   0,   0,
	0,              0,  SP_COPY,   0, '~',   0,   0,   0,
};

byte lastbutton = 0xff;
byte CheckButtons()
{
	byte x;
	byte y;
	byte i = 0;
	for(x = 0x80; x != 0; x = x >> 1)
	{
		deref(0xf046) = x;
		for(y = 0x80; y != 0; y = y >> 1)
		{
			if((deref(0xf040) & y) == 0)
			{
				if(i != lastbutton)
				{
					lastbutton = i;
					return i;
				}
				return 0xff;
			}
			++i;
		}
	}
	lastbutton = 0x50;
	return 0xff;
}

void draw_bytel(byte by,byte x, byte y)
{
	word a = 0x9000+(x>>3)+((word)y<<5);
	deref(a) = (by>>(x&7));
	deref(a+1) = (by<<(8-(x&7)));
	deref(a-1) = 0;
}


void draw_bytem(byte by,byte x, byte y)
{
	word a = 0x9800+(x>>3)+((word)y<<5);
	deref(a) = (by>>(x&7));
	deref(a+1) = (by<<(8-(x&7)));
	deref(a-1) = 0;
}

void draw_char(byte id, byte x, byte y)
{
	word i = 0;
	word a = (0x9000+(x>>3)+((y-1)<<5));
	word b = (0x9000+(x>>3)+((y+8)<<5));
	deref(a) = 0;
	deref(a+0x800) = 0;
	deref(b) = 0;
	deref(b+0x800) = 0;
	deref(a+1) = 0;
	deref(a+0x801) = 0;
	deref(b+1) = 0;
	deref(b+0x801) = 0;
	for (i=0;i<8;i++)
	{
		draw_bytel(light[((word)id<<3)+i],x,y+i);
		draw_bytem(dark[((word)id<<3)+i],x,y+i);
	}
	draw_map();
}

const byte wint[] =  "You Win!!!!!";
const byte loset[] = "You Lose!!!!";

void win()
{
	print(wint,5,1,2);
}
void lose()
{
	print(loset,5,1,2);
}

byte get_pixel(byte x, byte y)
{
	y++;
	if (x > 191 || y > 63 || y == 0)
	{
		while(1)
		lose();
	}
    return(val(((word)y<<5)+(x>>3)+0xF800)&(0x80>>(x&7)))?1:0;
}

void main()
{
	word i = 0;
	byte key = 0;
	byte cx,cy,dx,dy,cchar;
	byte up,down,left,right;
	//(0xf09C) = 0x01;
	//deref(0xf09C) = 0x01;
	val(0xf037) = 0;
	for (i=0xF800;i<0xffff;i++)
	{
		val(i) = 0;
	}
	val(0xf037) = 4;
	for (i=0xF800;i<0xffff;i++)
	{
		val(i) = 0;
	}
	for (i=0x9000;i<0xB000;i++)
	{
		val(i) = 0;
	}
	cx = 91;
	cy = 27;
	dx = 0;
	dy = 0;
	cchar = 0;
	draw_char(cchar,cx,cy);
	while (1)
	{
		if (!dx && !dy)
		{
			key = CheckButtons();
			if (key == 0xff)
			{
				continue;
			}
			switch(key)
			{
				case SP_UP:
					dx = 0;
					dy = -1;
					break;
				case SP_DOWN:
					dx = 0;
					dy = 1;
					break;
				case SP_LEFT:
					dx = -1;
					dy = 0;
					break;
				case SP_RIGHT:
					dx = 1;
					dy = 0;
					break;
				case SP_EXE:
					deref(0xF09C) ^= 1;
			}
		}
		left = get_pixel(cx-1,cy);
		right = get_pixel(cx+8,cy);
		up = get_pixel(cx,cy-1);
		down = get_pixel(cx,cy+8);
		if (left && dx == 0xff) {
			dx = 0;
			cchar = 4;
		} else if (right && dx == 1) {
			dx = 0;
			cchar = 6;
		} else if (up && dy == 0xff) {
			dy = 0;
			cchar = 3;
		} else if (down && dy == 1) {
			dy = 0;
			cchar = 5;
		} else {
			cx += dx;
			cy += dy;
			if (dx)
			{
				cchar = 2;
			}
			if (dy)
			{
				cchar = 1;
			}
			/*if (dx == 1)
			{
				plot_line(cx,cy+1,cx,cy+6,0);
			} else if (dy == 1) {
				plot_line(cx+1,cy,cx+6,cy,0);
			} else if (dy == 0xff) {
				plot_line(cx+1,cy+7,cx+6,cy+7,0);
			}*/
		}

		//delay(10);
		draw_char(cchar,cx,cy);
		if(cchar > 2 && cchar < 7)
		{
			cchar = 0;
			delay(250);
			draw_char(cchar,cx,cy);
			if(cx == 75 && cy == 52)
			{
				delay(4000);
				draw_char(7,cx,cy);
				while (1)
				win();
			}
		}
	}
}
