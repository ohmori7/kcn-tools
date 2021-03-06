/* Terminal color palettes. */
/* TODO: We should probably autogenerate this using xterm's perl script. */

/* The 16 ANSI colors. */
static struct rgb palette16[] = {
#if defined(PALA)
	{0x00, 0x00, 0x00},
	{0x80, 0x00, 0x00},
	{0x00, 0x80, 0x00},
	{0x80, 0x80, 0x00},
	{0x00, 0x00, 0x80},
	{0x80, 0x00, 0x80},
	{0x00, 0x80, 0x80},
	{0xC0, 0xC0, 0xC0},
	{0x80, 0x80, 0x80},
	{0xff, 0x00, 0x00},
	{0x00, 0xff, 0x00},
	{0xff, 0xff, 0x00},
	{0x00, 0x00, 0xff},
	{0xff, 0x00, 0xff},
	{0x00, 0xff, 0xff},
	{0xff, 0xff, 0xff},
#elif defined(PALB)
	{0x00, 0x00, 0x00},
	{0xaa, 0x00, 0x00},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x55, 0x00},
	{0x00, 0x00, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0xaa, 0xaa},
	{0x55, 0x55, 0x55},
	{0xff, 0x55, 0x55},
	{0x55, 0xff, 0x55},
	{0xff, 0xff, 0x55},
	{0x55, 0x55, 0xff},
	{0xff, 0x55, 0xff},
	{0x55, 0xff, 0xff},
	{0xff, 0xff, 0xff},
#else
	{0x00, 0x00, 0x00}, /* 0 */
	{0x80, 0x00, 0x00}, /* 1 */
	{0x00, 0x80, 0x00}, /* 2 */
	{0xaa, 0x55, 0x00}, /* 3 */
	{0x00, 0x00, 0x80}, /* 4 */
	{0x80, 0x00, 0x80}, /* 5 */
	{0x00, 0x80, 0x80}, /* 6 */
	{0xaa, 0xaa, 0xaa}, /* 7 */
	{0x55, 0x55, 0x55}, /* 8 */
	{0xff, 0x55, 0x55}, /* 9 */
	{0x55, 0xff, 0x55}, /* 10 */
	{0xff, 0xff, 0x55}, /* 11 */
	{0x55, 0x55, 0xff}, /* 12 */
	{0xff, 0x55, 0xff}, /* 13 */
	{0x55, 0xff, 0xff}, /* 14 */
	{0xff, 0xff, 0xff}, /* 15 */
#endif
};

#ifdef CONFIG_88_COLORS
/* Regexp'd from Dickey's xterm 88colres.h file. */
/* Colors  0-16 are the ANSI colors (and the same as palette16[]).
 * Colors 16-79 are 4x4x4 color cubes
 * Colors 80-91 are a grayscale ramp (with black and white left out). */
static struct rgb palette88[] = {
	{0x00, 0x00, 0x00}, /* 0 */
	{0x80, 0x00, 0x00}, /* 1 */
	{0x00, 0x80, 0x00}, /* 2 */
	{0xaa, 0x55, 0x00}, /* 3 */
	{0x00, 0x00, 0x80}, /* 4 */
	{0x80, 0x00, 0x80}, /* 5 */
	{0x00, 0x80, 0x80}, /* 6 */
	{0xaa, 0xaa, 0xaa}, /* 7 */
	{0x55, 0x55, 0x55}, /* 8 */
	{0xff, 0x55, 0x55}, /* 9 */
	{0x55, 0xff, 0x55}, /* 10 */
	{0xff, 0xff, 0x55}, /* 11 */
	{0x55, 0x55, 0xff}, /* 12 */
	{0xff, 0x55, 0xff}, /* 13 */
	{0x55, 0xff, 0xff}, /* 14 */
	{0xff, 0xff, 0xff}, /* 15 */

	{0x00, 0x00, 0x00}, /* 16 */
	{0x00, 0x00, 0x8b}, /* 17 */
	{0x00, 0x00, 0xcd}, /* 18 */
	{0x00, 0x00, 0xff}, /* 19 */
	{0x00, 0x8b, 0x00}, /* 20 */
	{0x00, 0x8b, 0x8b}, /* 21 */
	{0x00, 0x8b, 0xcd}, /* 22 */
	{0x00, 0x8b, 0xff}, /* 23 */
	{0x00, 0xcd, 0x00}, /* 24 */
	{0x00, 0xcd, 0x8b}, /* 25 */
	{0x00, 0xcd, 0xcd}, /* 26 */
	{0x00, 0xcd, 0xff}, /* 27 */
	{0x00, 0xff, 0x00}, /* 28 */
	{0x00, 0xff, 0x8b}, /* 29 */
	{0x00, 0xff, 0xcd}, /* 30 */
	{0x00, 0xff, 0xff}, /* 31 */

	{0x8b, 0x00, 0x00}, /* 32 */
	{0x8b, 0x00, 0x8b}, /* 33 */
	{0x8b, 0x00, 0xcd}, /* 34 */
	{0x8b, 0x00, 0xff}, /* 35 */
	{0x8b, 0x8b, 0x00}, /* 36 */
	{0x8b, 0x8b, 0x8b}, /* 37 */
	{0x8b, 0x8b, 0xcd}, /* 38 */
	{0x8b, 0x8b, 0xff}, /* 39 */
	{0x8b, 0xcd, 0x00}, /* 40 */
	{0x8b, 0xcd, 0x8b}, /* 41 */
	{0x8b, 0xcd, 0xcd}, /* 42 */
	{0x8b, 0xcd, 0xff}, /* 43 */
	{0x8b, 0xff, 0x00}, /* 44 */
	{0x8b, 0xff, 0x8b}, /* 45 */
	{0x8b, 0xff, 0xcd}, /* 46 */
	{0x8b, 0xff, 0xff}, /* 47 */

	{0xcd, 0x00, 0x00}, /* 48 */
	{0xcd, 0x00, 0x8b}, /* 49 */
	{0xcd, 0x00, 0xcd}, /* 50 */
	{0xcd, 0x00, 0xff}, /* 51 */
	{0xcd, 0x8b, 0x00}, /* 52 */
	{0xcd, 0x8b, 0x8b}, /* 53 */
	{0xcd, 0x8b, 0xcd}, /* 54 */
	{0xcd, 0x8b, 0xff}, /* 55 */
	{0xcd, 0xcd, 0x00}, /* 56 */
	{0xcd, 0xcd, 0x8b}, /* 57 */
	{0xcd, 0xcd, 0xcd}, /* 58 */
	{0xcd, 0xcd, 0xff}, /* 59 */
	{0xcd, 0xff, 0x00}, /* 60 */
	{0xcd, 0xff, 0x8b}, /* 61 */
	{0xcd, 0xff, 0xcd}, /* 62 */
	{0xcd, 0xff, 0xff}, /* 63 */

	{0xff, 0x00, 0x00}, /* 64 */
	{0xff, 0x00, 0x8b}, /* 65 */
	{0xff, 0x00, 0xcd}, /* 66 */
	{0xff, 0x00, 0xff}, /* 67 */
	{0xff, 0x8b, 0x00}, /* 68 */
	{0xff, 0x8b, 0x8b}, /* 69 */
	{0xff, 0x8b, 0xcd}, /* 70 */
	{0xff, 0x8b, 0xff}, /* 71 */
	{0xff, 0xcd, 0x00}, /* 72 */
	{0xff, 0xcd, 0x8b}, /* 73 */
	{0xff, 0xcd, 0xcd}, /* 74 */
	{0xff, 0xcd, 0xff}, /* 75 */
	{0xff, 0xff, 0x00}, /* 76 */
	{0xff, 0xff, 0x8b}, /* 77 */
	{0xff, 0xff, 0xcd}, /* 78 */
	{0xff, 0xff, 0xff}, /* 79 */

	{0x2e, 0x2e, 0x2e}, /* 80 */
	{0x5c, 0x5c, 0x5c}, /* 81 */
	{0x73, 0x73, 0x73}, /* 82 */
	{0x8b, 0x8b, 0x8b}, /* 83 */
	{0xa2, 0xa2, 0xa2}, /* 84 */
	{0xb9, 0xb9, 0xb9}, /* 85 */
	{0xd0, 0xd0, 0xd0}, /* 86 */
	{0xe7, 0xe7, 0xe7}, /* 87 */
};
#endif

#ifdef CONFIG_256_COLORS
/* Regexp'd from Dickey's xterm 256colres.h file. */
/* Colors   0- 16 are the ANSI colors (and the same as palette16[]).
 * Colors  16-231 are a 6x6x6 color cube
 * Colors 231-255 are a grayscale ramp (with black and white left out). */
static struct rgb palette256[] = {
	{0x00, 0x00, 0x00}, /* 0 */
	{0x80, 0x00, 0x00}, /* 1 */
	{0x00, 0x80, 0x00}, /* 2 */
	{0xaa, 0x55, 0x00}, /* 3 */
	{0x00, 0x00, 0x80}, /* 4 */
	{0x80, 0x00, 0x80}, /* 5 */
	{0x00, 0x80, 0x80}, /* 6 */
	{0xaa, 0xaa, 0xaa}, /* 7 */
	{0x55, 0x55, 0x55}, /* 8 */
	{0xff, 0x55, 0x55}, /* 9 */
	{0x55, 0xff, 0x55}, /* 10 */
	{0xff, 0xff, 0x55}, /* 11 */
	{0x55, 0x55, 0xff}, /* 12 */
	{0xff, 0x55, 0xff}, /* 13 */
	{0x55, 0xff, 0xff}, /* 14 */
	{0xff, 0xff, 0xff}, /* 15 */

	{0x00, 0x00, 0x00}, /* 16 */
	{0x00, 0x00, 0x5f}, /* 17 */
	{0x00, 0x00, 0x87}, /* 18 */
	{0x00, 0x00, 0xaf}, /* 19 */
	{0x00, 0x00, 0xd7}, /* 20 */
	{0x00, 0x00, 0xff}, /* 21 */
	{0x00, 0x5f, 0x00}, /* 22 */
	{0x00, 0x5f, 0x5f}, /* 23 */
	{0x00, 0x5f, 0x87}, /* 24 */
	{0x00, 0x5f, 0xaf}, /* 25 */
	{0x00, 0x5f, 0xd7}, /* 26 */
	{0x00, 0x5f, 0xff}, /* 27 */
	{0x00, 0x87, 0x00}, /* 28 */
	{0x00, 0x87, 0x5f}, /* 29 */
	{0x00, 0x87, 0x87}, /* 30 */
	{0x00, 0x87, 0xaf}, /* 31 */
	{0x00, 0x87, 0xd7}, /* 32 */
	{0x00, 0x87, 0xff}, /* 33 */
	{0x00, 0xaf, 0x00}, /* 34 */
	{0x00, 0xaf, 0x5f}, /* 35 */
	{0x00, 0xaf, 0x87}, /* 36 */
	{0x00, 0xaf, 0xaf}, /* 37 */
	{0x00, 0xaf, 0xd7}, /* 38 */
	{0x00, 0xaf, 0xff}, /* 39 */
	{0x00, 0xd7, 0x00}, /* 40 */
	{0x00, 0xd7, 0x5f}, /* 41 */
	{0x00, 0xd7, 0x87}, /* 42 */
	{0x00, 0xd7, 0xaf}, /* 43 */
	{0x00, 0xd7, 0xd7}, /* 44 */
	{0x00, 0xd7, 0xff}, /* 45 */
	{0x00, 0xff, 0x00}, /* 46 */
	{0x00, 0xff, 0x5f}, /* 47 */
	{0x00, 0xff, 0x87}, /* 48 */
	{0x00, 0xff, 0xaf}, /* 49 */
	{0x00, 0xff, 0xd7}, /* 50 */
	{0x00, 0xff, 0xff}, /* 51 */

	{0x5f, 0x00, 0x00}, /* 52 */
	{0x5f, 0x00, 0x5f}, /* 53 */
	{0x5f, 0x00, 0x87}, /* 54 */
	{0x5f, 0x00, 0xaf}, /* 55 */
	{0x5f, 0x00, 0xd7}, /* 56 */
	{0x5f, 0x00, 0xff}, /* 57 */
	{0x5f, 0x5f, 0x00}, /* 58 */
	{0x5f, 0x5f, 0x5f}, /* 59 */
	{0x5f, 0x5f, 0x87}, /* 60 */
	{0x5f, 0x5f, 0xaf}, /* 61 */
	{0x5f, 0x5f, 0xd7}, /* 62 */
	{0x5f, 0x5f, 0xff}, /* 63 */
	{0x5f, 0x87, 0x00}, /* 64 */
	{0x5f, 0x87, 0x5f}, /* 65 */
	{0x5f, 0x87, 0x87}, /* 66 */
	{0x5f, 0x87, 0xaf}, /* 67 */
	{0x5f, 0x87, 0xd7}, /* 68 */
	{0x5f, 0x87, 0xff}, /* 69 */
	{0x5f, 0xaf, 0x00}, /* 70 */
	{0x5f, 0xaf, 0x5f}, /* 71 */
	{0x5f, 0xaf, 0x87}, /* 72 */
	{0x5f, 0xaf, 0xaf}, /* 73 */
	{0x5f, 0xaf, 0xd7}, /* 74 */
	{0x5f, 0xaf, 0xff}, /* 75 */
	{0x5f, 0xd7, 0x00}, /* 76 */
	{0x5f, 0xd7, 0x5f}, /* 77 */
	{0x5f, 0xd7, 0x87}, /* 78 */
	{0x5f, 0xd7, 0xaf}, /* 79 */
	{0x5f, 0xd7, 0xd7}, /* 80 */
	{0x5f, 0xd7, 0xff}, /* 81 */
	{0x5f, 0xff, 0x00}, /* 82 */
	{0x5f, 0xff, 0x5f}, /* 83 */
	{0x5f, 0xff, 0x87}, /* 84 */
	{0x5f, 0xff, 0xaf}, /* 85 */
	{0x5f, 0xff, 0xd7}, /* 86 */
	{0x5f, 0xff, 0xff}, /* 87 */

	{0x87, 0x00, 0x00}, /* 88 */
	{0x87, 0x00, 0x5f}, /* 89 */
	{0x87, 0x00, 0x87}, /* 90 */
	{0x87, 0x00, 0xaf}, /* 91 */
	{0x87, 0x00, 0xd7}, /* 92 */
	{0x87, 0x00, 0xff}, /* 93 */
	{0x87, 0x5f, 0x00}, /* 94 */
	{0x87, 0x5f, 0x5f}, /* 95 */
	{0x87, 0x5f, 0x87}, /* 96 */
	{0x87, 0x5f, 0xaf}, /* 97 */
	{0x87, 0x5f, 0xd7}, /* 98 */
	{0x87, 0x5f, 0xff}, /* 99 */
	{0x87, 0x87, 0x00}, /* 100 */
	{0x87, 0x87, 0x5f}, /* 101 */
	{0x87, 0x87, 0x87}, /* 102 */
	{0x87, 0x87, 0xaf}, /* 103 */
	{0x87, 0x87, 0xd7}, /* 104 */
	{0x87, 0x87, 0xff}, /* 105 */
	{0x87, 0xaf, 0x00}, /* 106 */
	{0x87, 0xaf, 0x5f}, /* 107 */
	{0x87, 0xaf, 0x87}, /* 108 */
	{0x87, 0xaf, 0xaf}, /* 109 */
	{0x87, 0xaf, 0xd7}, /* 110 */
	{0x87, 0xaf, 0xff}, /* 111 */
	{0x87, 0xd7, 0x00}, /* 112 */
	{0x87, 0xd7, 0x5f}, /* 113 */
	{0x87, 0xd7, 0x87}, /* 114 */
	{0x87, 0xd7, 0xaf}, /* 115 */
	{0x87, 0xd7, 0xd7}, /* 116 */
	{0x87, 0xd7, 0xff}, /* 117 */
	{0x87, 0xff, 0x00}, /* 118 */
	{0x87, 0xff, 0x5f}, /* 119 */
	{0x87, 0xff, 0x87}, /* 120 */
	{0x87, 0xff, 0xaf}, /* 121 */
	{0x87, 0xff, 0xd7}, /* 122 */
	{0x87, 0xff, 0xff}, /* 123 */

	{0xaf, 0x00, 0x00}, /* 124 */
	{0xaf, 0x00, 0x5f}, /* 125 */
	{0xaf, 0x00, 0x87}, /* 126 */
	{0xaf, 0x00, 0xaf}, /* 127 */
	{0xaf, 0x00, 0xd7}, /* 128 */
	{0xaf, 0x00, 0xff}, /* 129 */
	{0xaf, 0x5f, 0x00}, /* 130 */
	{0xaf, 0x5f, 0x5f}, /* 131 */
	{0xaf, 0x5f, 0x87}, /* 132 */
	{0xaf, 0x5f, 0xaf}, /* 133 */
	{0xaf, 0x5f, 0xd7}, /* 134 */
	{0xaf, 0x5f, 0xff}, /* 135 */
	{0xaf, 0x87, 0x00}, /* 136 */
	{0xaf, 0x87, 0x5f}, /* 137 */
	{0xaf, 0x87, 0x87}, /* 138 */
	{0xaf, 0x87, 0xaf}, /* 139 */
	{0xaf, 0x87, 0xd7}, /* 140 */
	{0xaf, 0x87, 0xff}, /* 141 */
	{0xaf, 0xaf, 0x00}, /* 142 */
	{0xaf, 0xaf, 0x5f}, /* 143 */
	{0xaf, 0xaf, 0x87}, /* 144 */
	{0xaf, 0xaf, 0xaf}, /* 145 */
	{0xaf, 0xaf, 0xd7}, /* 146 */
	{0xaf, 0xaf, 0xff}, /* 147 */
	{0xaf, 0xd7, 0x00}, /* 148 */
	{0xaf, 0xd7, 0x5f}, /* 149 */
	{0xaf, 0xd7, 0x87}, /* 150 */
	{0xaf, 0xd7, 0xaf}, /* 151 */
	{0xaf, 0xd7, 0xd7}, /* 152 */
	{0xaf, 0xd7, 0xff}, /* 153 */
	{0xaf, 0xff, 0x00}, /* 154 */
	{0xaf, 0xff, 0x5f}, /* 155 */
	{0xaf, 0xff, 0x87}, /* 156 */
	{0xaf, 0xff, 0xaf}, /* 157 */
	{0xaf, 0xff, 0xd7}, /* 158 */
	{0xaf, 0xff, 0xff}, /* 159 */

	{0xd7, 0x00, 0x00}, /* 160 */
	{0xd7, 0x00, 0x5f}, /* 161 */
	{0xd7, 0x00, 0x87}, /* 162 */
	{0xd7, 0x00, 0xaf}, /* 163 */
	{0xd7, 0x00, 0xd7}, /* 164 */
	{0xd7, 0x00, 0xff}, /* 165 */
	{0xd7, 0x5f, 0x00}, /* 166 */
	{0xd7, 0x5f, 0x5f}, /* 167 */
	{0xd7, 0x5f, 0x87}, /* 168 */
	{0xd7, 0x5f, 0xaf}, /* 169 */
	{0xd7, 0x5f, 0xd7}, /* 170 */
	{0xd7, 0x5f, 0xff}, /* 171 */
	{0xd7, 0x87, 0x00}, /* 172 */
	{0xd7, 0x87, 0x5f}, /* 173 */
	{0xd7, 0x87, 0x87}, /* 174 */
	{0xd7, 0x87, 0xaf}, /* 175 */
	{0xd7, 0x87, 0xd7}, /* 176 */
	{0xd7, 0x87, 0xff}, /* 177 */
	{0xd7, 0xaf, 0x00}, /* 178 */
	{0xd7, 0xaf, 0x5f}, /* 179 */
	{0xd7, 0xaf, 0x87}, /* 180 */
	{0xd7, 0xaf, 0xaf}, /* 181 */
	{0xd7, 0xaf, 0xd7}, /* 182 */
	{0xd7, 0xaf, 0xff}, /* 183 */
	{0xd7, 0xd7, 0x00}, /* 184 */
	{0xd7, 0xd7, 0x5f}, /* 185 */
	{0xd7, 0xd7, 0x87}, /* 186 */
	{0xd7, 0xd7, 0xaf}, /* 187 */
	{0xd7, 0xd7, 0xd7}, /* 188 */
	{0xd7, 0xd7, 0xff}, /* 189 */
	{0xd7, 0xff, 0x00}, /* 190 */
	{0xd7, 0xff, 0x5f}, /* 191 */
	{0xd7, 0xff, 0x87}, /* 192 */
	{0xd7, 0xff, 0xaf}, /* 193 */
	{0xd7, 0xff, 0xd7}, /* 194 */
	{0xd7, 0xff, 0xff}, /* 195 */

	{0xff, 0x00, 0x00}, /* 196 */
	{0xff, 0x00, 0x5f}, /* 197 */
	{0xff, 0x00, 0x87}, /* 198 */
	{0xff, 0x00, 0xaf}, /* 199 */
	{0xff, 0x00, 0xd7}, /* 200 */
	{0xff, 0x00, 0xff}, /* 201 */
	{0xff, 0x5f, 0x00}, /* 202 */
	{0xff, 0x5f, 0x5f}, /* 203 */
	{0xff, 0x5f, 0x87}, /* 204 */
	{0xff, 0x5f, 0xaf}, /* 205 */
	{0xff, 0x5f, 0xd7}, /* 206 */
	{0xff, 0x5f, 0xff}, /* 207 */
	{0xff, 0x87, 0x00}, /* 208 */
	{0xff, 0x87, 0x5f}, /* 209 */
	{0xff, 0x87, 0x87}, /* 210 */
	{0xff, 0x87, 0xaf}, /* 211 */
	{0xff, 0x87, 0xd7}, /* 212 */
	{0xff, 0x87, 0xff}, /* 213 */
	{0xff, 0xaf, 0x00}, /* 214 */
	{0xff, 0xaf, 0x5f}, /* 215 */
	{0xff, 0xaf, 0x87}, /* 216 */
	{0xff, 0xaf, 0xaf}, /* 217 */
	{0xff, 0xaf, 0xd7}, /* 218 */
	{0xff, 0xaf, 0xff}, /* 219 */
	{0xff, 0xd7, 0x00}, /* 220 */
	{0xff, 0xd7, 0x5f}, /* 221 */
	{0xff, 0xd7, 0x87}, /* 222 */
	{0xff, 0xd7, 0xaf}, /* 223 */
	{0xff, 0xd7, 0xd7}, /* 224 */
	{0xff, 0xd7, 0xff}, /* 225 */
	{0xff, 0xff, 0x00}, /* 226 */
	{0xff, 0xff, 0x5f}, /* 227 */
	{0xff, 0xff, 0x87}, /* 228 */
	{0xff, 0xff, 0xaf}, /* 229 */
	{0xff, 0xff, 0xd7}, /* 230 */
	{0xff, 0xff, 0xff}, /* 231 */

	{0x08, 0x08, 0x08}, /* 232 */
	{0x12, 0x12, 0x12}, /* 233 */
	{0x1c, 0x1c, 0x1c}, /* 234 */
	{0x26, 0x26, 0x26}, /* 235 */
	{0x30, 0x30, 0x30}, /* 236 */
	{0x3a, 0x3a, 0x3a}, /* 237 */
	{0x44, 0x44, 0x44}, /* 238 */
	{0x4e, 0x4e, 0x4e}, /* 239 */
	{0x58, 0x58, 0x58}, /* 240 */
	{0x62, 0x62, 0x62}, /* 241 */
	{0x6c, 0x6c, 0x6c}, /* 242 */
	{0x76, 0x76, 0x76}, /* 243 */
	{0x80, 0x80, 0x80}, /* 244 */
	{0x8a, 0x8a, 0x8a}, /* 245 */
	{0x94, 0x94, 0x94}, /* 246 */
	{0x9e, 0x9e, 0x9e}, /* 247 */
	{0xa8, 0xa8, 0xa8}, /* 248 */
	{0xb2, 0xb2, 0xb2}, /* 249 */
	{0xbc, 0xbc, 0xbc}, /* 250 */
	{0xc6, 0xc6, 0xc6}, /* 251 */
	{0xd0, 0xd0, 0xd0}, /* 252 */
	{0xda, 0xda, 0xda}, /* 253 */
	{0xe4, 0xe4, 0xe4}, /* 254 */
	{0xee, 0xee, 0xee}, /* 255 */
};
#endif

/* vim: set filetype=c : */
