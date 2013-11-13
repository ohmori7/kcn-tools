/* Charsets convertor */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#include <ctype.h>
#include <stdlib.h>

#include "elinks.h"

#include "document/options.h"
#include "intl/charsets.h"
#include "util/conv.h"
#include "util/error.h"
#include "util/fastfind.h"
#include "util/memory.h"
#include "util/string.h"


/* Fix namespace clash on MacOS. */
#define table table_elinks

struct table_entry {
	unsigned char c;
	unicode_val_T u;
};

struct codepage_desc {
	unsigned char *name;
	unsigned char **aliases;
	struct table_entry *table;
};

#include "intl/codepage.inc"
#include "intl/uni_7b.inc"
#include "intl/entity.inc"


static char strings[256][2] = {
	"\000", "\001", "\002", "\003", "\004", "\005", "\006", "\007",
	"\010", "\011", "\012", "\013", "\014", "\015", "\016", "\017",
	"\020", "\021", "\022", "\023", "\024", "\025", "\026", "\033",
	"\030", "\031", "\032", "\033", "\034", "\035", "\036", "\033",
	"\040", "\041", "\042", "\043", "\044", "\045", "\046", "\047",
	"\050", "\051", "\052", "\053", "\054", "\055", "\056", "\057",
	"\060", "\061", "\062", "\063", "\064", "\065", "\066", "\067",
	"\070", "\071", "\072", "\073", "\074", "\075", "\076", "\077",
	"\100", "\101", "\102", "\103", "\104", "\105", "\106", "\107",
	"\110", "\111", "\112", "\113", "\114", "\115", "\116", "\117",
	"\120", "\121", "\122", "\123", "\124", "\125", "\126", "\127",
	"\130", "\131", "\132", "\133", "\134", "\135", "\136", "\137",
	"\140", "\141", "\142", "\143", "\144", "\145", "\146", "\147",
	"\150", "\151", "\152", "\153", "\154", "\155", "\156", "\157",
	"\160", "\161", "\162", "\163", "\164", "\165", "\166", "\167",
	"\170", "\171", "\172", "\173", "\174", "\175", "\176", "\177",
	"\200", "\201", "\202", "\203", "\204", "\205", "\206", "\207",
	"\210", "\211", "\212", "\213", "\214", "\215", "\216", "\217",
	"\220", "\221", "\222", "\223", "\224", "\225", "\226", "\227",
	"\230", "\231", "\232", "\233", "\234", "\235", "\236", "\237",
	"\240", "\241", "\242", "\243", "\244", "\245", "\246", "\247",
	"\250", "\251", "\252", "\253", "\254", "\255", "\256", "\257",
	"\260", "\261", "\262", "\263", "\264", "\265", "\266", "\267",
	"\270", "\271", "\272", "\273", "\274", "\275", "\276", "\277",
	"\300", "\301", "\302", "\303", "\304", "\305", "\306", "\307",
	"\310", "\311", "\312", "\313", "\314", "\315", "\316", "\317",
	"\320", "\321", "\322", "\323", "\324", "\325", "\326", "\327",
	"\330", "\331", "\332", "\333", "\334", "\335", "\336", "\337",
	"\340", "\341", "\342", "\343", "\344", "\345", "\346", "\347",
	"\350", "\351", "\352", "\353", "\354", "\355", "\356", "\357",
	"\360", "\361", "\362", "\363", "\364", "\365", "\366", "\367",
	"\370", "\371", "\372", "\373", "\374", "\375", "\376", "\377",
};

static void
free_translation_table(struct conv_table *p)
{
	int i;

	for (i = 0; i < 256; i++)
		if (p[i].t)
			free_translation_table(p[i].u.tbl);

	mem_free(p);
}

static unsigned char *no_str = "*";

static void
new_translation_table(struct conv_table *p)
{
	int i;

	for (i = 0; i < 256; i++)
		if (p[i].t)
			free_translation_table(p[i].u.tbl);
	for (i = 0; i < 128; i++) {
		p[i].t = 0;
		p[i].u.str = strings[i];
	}
	for (; i < 256; i++) {
		p[i].t = 0;
	       	p[i].u.str = no_str;
	}
}

#define BIN_SEARCH(table, entry, entries, key, result)					\
{											\
	long _s = 0, _e = (entries) - 1;						\
											\
	while (_s <= _e || !((result) = -1)) {						\
		long _m = (_s + _e) / 2;						\
											\
		if ((table)[_m].entry == (key)) {					\
			(result) = _m;							\
			break;								\
		}									\
		if ((table)[_m].entry > (key)) _e = _m - 1;				\
		if ((table)[_m].entry < (key)) _s = _m + 1;				\
	}										\
}											\

static const unicode_val_T strange_chars[32] = {
0x20ac, 0x0000, 0x002a, 0x0000, 0x201e, 0x2026, 0x2020, 0x2021,
0x005e, 0x2030, 0x0160, 0x003c, 0x0152, 0x0000, 0x0000, 0x0000,
0x0000, 0x0060, 0x0027, 0x0022, 0x0022, 0x002a, 0x2013, 0x2014,
0x007e, 0x2122, 0x0161, 0x003e, 0x0153, 0x0000, 0x0000, 0x0000,
};

#define SYSTEM_CHARSET_FLAG 128

unsigned char *
u2cp_(unicode_val_T u, int to, int no_nbsp_hack)
{
	int j;
	int s;

	if (u < 128) return strings[u];
	/* To mark non breaking spaces, we use a special char NBSP_CHAR. */
	if (u == UCS_NO_BREAK_SPACE)
		return no_nbsp_hack ? " " : NBSP_CHAR_STRING;
	if (u == UCS_SOFT_HYPHEN) return "";

	if (u < 0xa0) {
		unicode_val_T strange = strange_chars[u - 0x80];

		if (!strange) return NULL;
		return u2cp_(strange, to, no_nbsp_hack);
	}

	to &= ~SYSTEM_CHARSET_FLAG;

	for (j = 0; codepages[to].table[j].c; j++)
		if (codepages[to].table[j].u == u)
			return strings[codepages[to].table[j].c];

	BIN_SEARCH(unicode_7b, x, N_UNICODE_7B, u, s);
	if (s != -1) return unicode_7b[s].s;

	return no_str;
}

static unsigned char utf_buffer[7];

static unsigned char *
encode_utf_8(unicode_val_T u)
{
	memset(utf_buffer, 0, 7);

	if (u < 0x80)
		utf_buffer[0] = u;
	else if (u < 0x800)
		utf_buffer[0] = 0xc0 | ((u >> 6) & 0x1f),
		utf_buffer[1] = 0x80 | (u & 0x3f);
	else if (u < 0x10000)
		utf_buffer[0] = 0xe0 | ((u >> 12) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[2] = 0x80 | (u & 0x3f);
	else if (u < 0x200000)
		utf_buffer[0] = 0xf0 | ((u >> 18) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[3] = 0x80 | (u & 0x3f);
	else if (u < 0x4000000)
		utf_buffer[0] = 0xf8 | ((u >> 24) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 18) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[3] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[4] = 0x80 | (u & 0x3f);
	else	utf_buffer[0] = 0xfc | ((u >> 30) & 0x01),
		utf_buffer[1] = 0x80 | ((u >> 24) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 18) & 0x3f),
		utf_buffer[3] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[4] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[5] = 0x80 | (u & 0x3f);

	return utf_buffer;
}

/* This slow and ugly code is used by the terminal utf_8_io */
unsigned char *
cp2utf_8(int from, int c)
{
	int j;

	from &= ~SYSTEM_CHARSET_FLAG;

	if (codepages[from].table == table_utf_8 || c < 128)
		return strings[c];

	for (j = 0; codepages[from].table[j].c; j++)
		if (codepages[from].table[j].c == c)
			return encode_utf_8(codepages[from].table[j].u);

	return encode_utf_8(UCS_NO_CHAR);
}

static void
add_utf_8(struct conv_table *ct, unicode_val_T u, unsigned char *str)
{
	unsigned char *p = encode_utf_8(u);

	while (p[1]) {
		if (ct[*p].t) ct = ct[*p].u.tbl;
		else {
			struct conv_table *nct;

			assertm(ct[*p].u.str == no_str, "bad utf encoding #1");
			if_assert_failed return;

			nct = mem_calloc(256, sizeof(*nct));
			if (!nct) return;
			new_translation_table(nct);
			ct[*p].t = 1;
			ct[*p].u.tbl = nct;
			ct = nct;
		}
		p++;
	}

	assertm(!ct[*p].t, "bad utf encoding #2");
	if_assert_failed return;

	if (ct[*p].u.str == no_str)
		ct[*p].u.str = str;
}

struct conv_table utf_table[256];
int utf_table_init = 1;

static void
free_utf_table(void)
{
	int i;

	for (i = 128; i < 256; i++)
		mem_free(utf_table[i].u.str);
}

static struct conv_table *
get_translation_table_to_utf_8(int from)
{
	int i;
	static int lfr = -1;

	if (from == -1) return NULL;
	from &= ~SYSTEM_CHARSET_FLAG;
	if (from == lfr) return utf_table;
	if (utf_table_init)
		memset(utf_table, 0, sizeof(utf_table)),
		utf_table_init = 0;
	else
		free_utf_table();

	for (i = 0; i < 128; i++)
		utf_table[i].u.str = strings[i];

	if (codepages[from].table == table_utf_8) {
		for (i = 128; i < 256; i++)
			utf_table[i].u.str = stracpy(strings[i]);
		return utf_table;
	}

	for (i = 128; i < 256; i++)
		utf_table[i].u.str = NULL;

	for (i = 0; codepages[from].table[i].c; i++) {
		unicode_val_T u = codepages[from].table[i].u;

		if (!utf_table[codepages[from].table[i].c].u.str)
			utf_table[codepages[from].table[i].c].u.str =
				stracpy(encode_utf_8(u));
	}

	for (i = 128; i < 256; i++)
		if (!utf_table[i].u.str)
			utf_table[i].u.str = stracpy(no_str);

	return utf_table;
}

struct conv_table table[256];
static int first = 1;

void
free_conv_table(void)
{
	if (!utf_table_init) free_utf_table();
	if (first) {
		memset(table, 0, sizeof(table));
		first = 0;
	}
	new_translation_table(table);
}


struct conv_table *
get_translation_table(int from, int to)
{
	static int lfr = -1;
	static int lto = -1;

	from &= ~SYSTEM_CHARSET_FLAG;
	to &= ~SYSTEM_CHARSET_FLAG;
	if (first) {
		memset(table, 0, sizeof(table));
		first = 0;
	}
	if (/*from == to ||*/ from == -1 || to == -1)
		return NULL;
	if (codepages[to].table == table_utf_8)
		return get_translation_table_to_utf_8(from);
	if (from == lfr && to == lto)
		return table;
	lfr = from;
	lto = to;
	new_translation_table(table);

	if (codepages[from].table == table_utf_8) {
		int i;

		/* Map U+00A0 and U+00AD the same way as u2cp() would.  */
		add_utf_8(table, UCS_NO_BREAK_SPACE, strings[NBSP_CHAR]);
		add_utf_8(table, UCS_SOFT_HYPHEN, "");

		for (i = 0; codepages[to].table[i].c; i++)
			add_utf_8(table, codepages[to].table[i].u,
				  strings[codepages[to].table[i].c]);

		for (i = 0; unicode_7b[i].x != -1; i++)
			if (unicode_7b[i].x >= 0x80)
				add_utf_8(table, unicode_7b[i].x,
					  unicode_7b[i].s);

	} else {
		int i;

		for (i = 128; i < 256; i++) {
			int j;

			for (j = 0; codepages[from].table[j].c; j++) {
				if (codepages[from].table[j].c == i) {
					unsigned char *u;

					u = u2cp(codepages[from].table[j].u, to);
					if (u) table[i].u.str = u;
					break;
				}
			}
		}
	}

	return table;
}

static inline int
xxstrcmp(unsigned char *s1, unsigned char *s2, int l2)
{
	while (l2) {
		if (*s1 > *s2) return 1;
		if (*s1 < *s2) return -1;
		s1++;
	       	s2++;
		l2--;
	}

	return *s2 ? -1 : 0;
}

/* Entity cache debugging purpose. */
#if 0
#define DEBUG_ENTITY_CACHE
#else
#undef DEBUG_ENTITY_CACHE
#endif

struct entity_cache {
	unsigned int hits;
	int strlen;
	int encoding;
	unsigned char *result;
	unsigned char str[20]; /* Suffice in any case. */
};

static int
hits_cmp(struct entity_cache *a, struct entity_cache *b)
{
	if (a->hits == b->hits) return 0;
	if (a->hits > b->hits) return -1;
	else return 1;
}

static int
compare_entities(const void *key_, const void *element_)
{
	struct string *key = (struct string *) key_;
	struct entity *element = (struct entity *) element_;
	int length = key->length;
	unsigned char *first = key->source;
	unsigned char *second = element->s;

	return xxstrcmp(first, second, length);
}

unsigned char *
get_entity_string(const unsigned char *str, const int strlen, int encoding)
{
#define ENTITY_CACHE_SIZE 10	/* 10 seems a good value. */
#define ENTITY_CACHE_MAXLEN 9   /* entities with length >= ENTITY_CACHE_MAXLEN or == 1
			           will go in [0] table */
	static struct entity_cache entity_cache[ENTITY_CACHE_MAXLEN][ENTITY_CACHE_SIZE];
	static unsigned int nb_entity_cache[ENTITY_CACHE_MAXLEN];
	static int first_time = 1;
	unsigned int slen;
	unsigned char *result = NULL;

	if (strlen <= 0) return NULL;

	if (first_time) {
		memset(&nb_entity_cache, 0, ENTITY_CACHE_MAXLEN * sizeof(unsigned int));
		first_time = 0;
	}

	/* Check if cached. A test on many websites (freshmeat.net + whole ELinks website
	 * + google + slashdot + websites that result from a search for test on google,
	 * + various ones) show quite impressive improvment:
	 * Top ten is:
	 * 0: hits=2459 l=4 st='nbsp'
	 * 1: hits=2152 l=6 st='eacute'
	 * 2: hits=235 l=6 st='egrave'
	 * 3: hits=136 l=6 st='agrave'
	 * 4: hits=100 l=3 st='amp'
	 * 5: hits=40 l=5 st='laquo'
	 * 6: hits=8 l=4 st='copy'
	 * 7: hits=5 l=2 st='gt'
	 * 8: hits=2 l=2 st='lt'
	 * 9: hits=1 l=6 st='middot'
	 *
	 * Most of the time cache hit ratio is near 95%.
	 *
	 * A long test shows: 15186 hits vs. 24 misses and mean iteration
	 * count is kept < 2 (worst case 1.58). Not so bad ;)
	 *
	 * --Zas */

	/* entities with length >= ENTITY_CACHE_MAXLEN or == 1 will go in [0] table */
	slen = (strlen > 1 && strlen < ENTITY_CACHE_MAXLEN) ? strlen : 0;

	if (strlen < ENTITY_CACHE_MAXLEN && nb_entity_cache[slen] > 0) {
		int i;

		for (i = 0; i < nb_entity_cache[slen]; i++) {
			if (entity_cache[slen][i].encoding == encoding
			    && !memcmp(str, entity_cache[slen][i].str, strlen)) {
#ifdef DEBUG_ENTITY_CACHE
				static double total_iter = 0;
				static unsigned long hit_count = 0;

				total_iter += i + 1;
				hit_count++;
				fprintf(stderr, "hit after %d iter. (mean = %0.2f)\n", i + 1, total_iter / (double) hit_count);
#endif
				if (entity_cache[slen][i].hits < (unsigned int) ~0)
					entity_cache[slen][i].hits++;
				return entity_cache[slen][i].result;
			}
		}
#ifdef DEBUG_ENTITY_CACHE
		fprintf(stderr, "miss\n");
#endif
	}

	if (*str == '#') { /* Numeric entity. */
		int l = (int) strlen;
		unsigned char *st = (unsigned char *) str;
		unicode_val_T n = 0;

		if (l == 1) goto end; /* &#; ? */
		st++, l--;
		if ((*st | 32) == 'x') { /* Hexadecimal */

			if (l == 1 || l > 9) goto end; /* xFFFFFFFF max. */
			st++, l--;
			do {
				unsigned char c = (*(st++) | 32);

				if (isdigit(c))
					n = (n << 4) | (c - '0');
				else if (isxdigit(c))
					n = (n << 4) | (c - 'a' + 10);
				else
					goto end; /* Bad char. */
			} while (--l);
		} else { /* Decimal */
			if (l > 10) goto end; /* 4294967295 max. */
			do {
				unsigned char c = *(st++);

				if (isdigit(c))
					n = n * 10 + c - '0';
				else
					goto end; /* Bad char. */
				/* Limit to 0xFFFFFFFF. */
				if (n >= (unicode_val_T) 0xFFFFFFFFu)
					goto end;
			} while (--l);
		}

		result = u2cp(n, encoding);

#ifdef DEBUG_ENTITY_CACHE
		fprintf(stderr, "%lu %016x %s\n", (unsigned long) n , n, result);
#endif
	} else { /* Text entity. */
		struct string key = INIT_STRING((unsigned char *) str, strlen);
		struct entity *element = bsearch((void *) &key, entities,
						 N_ENTITIES,
						 sizeof(*element),
						 compare_entities);

		if (element) result = u2cp(element->c, encoding);
	}

end:
	/* Take care of potential buffer overflow. */
	if (strlen < sizeof(entity_cache[slen][0].str)) {
		struct entity_cache *ece;

		/* Sort entries by hit order. */
		if (nb_entity_cache[slen] > 1)
			qsort(&entity_cache[slen][0], nb_entity_cache[slen],
			      sizeof(entity_cache[slen][0]), (void *) hits_cmp);

		/* Increment number of cache entries if possible.
		 * Else, just replace the least used entry.  */
		if (nb_entity_cache[slen] < ENTITY_CACHE_SIZE) nb_entity_cache[slen]++;
		ece = &entity_cache[slen][nb_entity_cache[slen] - 1];

		/* Copy new entry to cache. */
		ece->hits = 1;
		ece->strlen = strlen;
		ece->encoding = encoding;
		ece->result = result;
		memcpy(ece->str, str, strlen);
		ece->str[strlen] = '\0';


#ifdef DEBUG_ENTITY_CACHE
		fprintf(stderr, "Added in [%u]: l=%d st='%s'\n", slen,
				entity_cache[slen][0].strlen, entity_cache[slen][0].str);

	{
		unsigned int i;

		fprintf(stderr, "- Cache entries [%u] -\n", slen);
		for (i = 0; i < nb_entity_cache[slen] ; i++)
			fprintf(stderr, "%d: hits=%u l=%d st='%s'\n", i,
				entity_cache[slen][i].hits, entity_cache[slen][i].strlen,
				entity_cache[slen][i].str);
		fprintf(stderr, "-----------------\n");
	}
#endif	/* DEBUG_ENTITY_CACHE */
	}
	return result;
}

unsigned char *
convert_string(struct conv_table *convert_table,
	       unsigned char *chars, int charslen, int cp,
	       enum convert_string_mode mode, int *length,
	       void (*callback)(void *data, unsigned char *buf, int buflen),
	       void *callback_data)
{
	unsigned char *buffer;
	int bufferpos = 0;
	int charspos = 0;

	if (!convert_table && !memchr(chars, '&', charslen)) {
		if (callback) {
			if (charslen) callback(callback_data, chars, charslen);
			return NULL;
		} else {
			return memacpy(chars, charslen);
		}
	}

	/* Buffer allocation */

	buffer = mem_alloc(ALLOC_GR + 1 /* trailing \0 */);
	if (!buffer) return NULL;

	/* Iterate ;-) */

	while (charspos < charslen) {
		unsigned char *translit;

#define PUTC do { \
		buffer[bufferpos++] = chars[charspos++]; \
		translit = ""; \
		goto flush; \
	} while (0)

		if (chars[charspos] != '&') {
			struct conv_table *t;
			int i;

			if (chars[charspos] < 128 || !convert_table) PUTC;

			t = convert_table;
			i = charspos;

			while (t[chars[i]].t) {
				t = t[chars[i++]].u.tbl;
				if (i >= charslen) PUTC;
			}

			translit = t[chars[i]].u.str;
			charspos = i + 1;

		} else if (mode == CSM_FORM || mode == CSM_NONE) {
			PUTC;

		} else {
			int start = charspos + 1;
			int i = start;

			while (i < charslen
			       && (isasciialpha(chars[i])
				   || isdigit(chars[i])
				   || (chars[i] == '#')))
				i++;

			/* This prevents bug 213: we were expanding "entities"
			 * in URL query strings. */
			/* XXX: But this disables &nbsp&nbsp usage, which
			 * appears to be relatively common! --pasky */
			if ((mode == CSM_DEFAULT || (chars[i] != '&' && chars[i] != '='))
			    && i > start
			    && !isasciialpha(chars[i]) && !isdigit(chars[i])) {
				translit = get_entity_string(&chars[start], i - start,
						      cp);
				if (chars[i] != ';') {
					/* Eat &nbsp &nbsp<foo> happily, but
					 * pull back from the character after
					 * entity string if it is not the valid
					 * terminator. */
					i--;
				}

				if (!translit) PUTC;
				charspos = i + (i < charslen);
			} else PUTC;
		}

		if (!translit[0]) continue;

		if (!translit[1]) {
			buffer[bufferpos++] = translit[0];
			translit = "";
			goto flush;
		}

		while (*translit) {
			unsigned char *new;

			buffer[bufferpos++] = *(translit++);
flush:
			if (bufferpos & (ALLOC_GR - 1)) continue;

			if (callback) {
				buffer[bufferpos] = 0;
				callback(callback_data, buffer, bufferpos);
				bufferpos = 0;
			} else {
				new = mem_realloc(buffer, bufferpos + ALLOC_GR);
				if (!new) {
					mem_free(buffer);
					return NULL;
				}
				buffer = new;
			}
		}
#undef PUTC
	}

	/* Say bye */

	buffer[bufferpos] = 0;
	if (length) *length = bufferpos;

	if (callback) {
		if (bufferpos) callback(callback_data, buffer, bufferpos);
		mem_free(buffer);
		return NULL;
	} else {
		return buffer;
	}
}


#ifndef USE_FASTFIND
int
get_cp_index(const unsigned char *name)
{
	int i, a;
	int syscp = 0;

	if (!c_strcasecmp(name, "System")) {
#if HAVE_LANGINFO_CODESET
		name = nl_langinfo(CODESET);
		syscp = SYSTEM_CHARSET_FLAG;
#else
		name = "us-ascii";
#endif
	}

	for (i = 0; codepages[i].name; i++) {
		for (a = 0; codepages[i].aliases[a]; a++) {
			/* In the past, we looked for the longest substring
			 * in all the names; it is way too expensive, though:
			 *
			 *   %   cumulative   self              self     total
			 *  time   seconds   seconds    calls  us/call  us/call  name
			 *  3.00      0.66     0.03     1325    22.64    22.64  get_cp_index
			 *
			 * Anything called from redraw_screen() is in fact
			 * relatively expensive, even if it's called just
			 * once. So we will do a simple strcasecmp() here.
			 */

			if (!c_strcasecmp(name, codepages[i].aliases[a]))
				return i | syscp;
		}
	}

	if (syscp) {
		return get_cp_index("us-ascii") | syscp;
	} else {
		return -1;
	}
}

#else

static unsigned int i_name = 0;
static unsigned int i_alias = 0;

/* Reset internal list pointer */
void
charsets_list_reset(void)
{
	i_name = 0;
	i_alias = 0;
}

/* Returns a pointer to a struct that contains current key and data pointers
 * and increment internal pointer.  It returns NULL when key is NULL. */
struct fastfind_key_value *
charsets_list_next(void)
{
	static struct fastfind_key_value kv;

	if (!codepages[i_name].name) return NULL;

	kv.key = codepages[i_name].aliases[i_alias];
	kv.data = &codepages[i_name];

	if (codepages[i_name].aliases[i_alias + 1])
		i_alias++;
	else {
		i_name++;
		i_alias = 0;
	}

	return &kv;
}

static struct fastfind_index ff_charsets_index
	= INIT_FASTFIND_INDEX("charsets_lookup", charsets_list_reset, charsets_list_next);

/* It searchs for a charset named @name or one of its aliases and
 * returns index for it or -1 if not found. */
int
get_cp_index(const unsigned char *name)
{
	struct codepage_desc *codepage;
	int syscp = 0;

	if (!c_strcasecmp(name, "System")) {
#if HAVE_LANGINFO_CODESET
		name = nl_langinfo(CODESET);
		syscp = SYSTEM_CHARSET_FLAG;
#else
		name = "us-ascii";
#endif
	}

	codepage = fastfind_search(&ff_charsets_index, name, strlen(name));
	if (codepage) {
		assert(codepages <= codepage && codepage < codepages + N_CODEPAGES);
		return (codepage - codepages) | syscp;

	} else if (syscp) {
		return get_cp_index("us-ascii") | syscp;

	} else {
		return -1;
	}
}

#endif /* USE_FASTFIND */

void
init_charsets_lookup(void)
{
#ifdef USE_FASTFIND
	fastfind_index(&ff_charsets_index, FF_COMPRESS);
#endif
}

void
free_charsets_lookup(void)
{
#ifdef USE_FASTFIND
	fastfind_done(&ff_charsets_index);
#endif
}

unsigned char *
get_cp_name(int cp_index)
{
	if (cp_index < 0) return "none";
	if (cp_index & SYSTEM_CHARSET_FLAG) return "System";

	return codepages[cp_index].name;
}

unsigned char *
get_cp_mime_name(int cp_index)
{
	if (cp_index < 0) return "none";
	if (cp_index & SYSTEM_CHARSET_FLAG) return "System";
	if (!codepages[cp_index].aliases) return NULL;

	return codepages[cp_index].aliases[0];
}

int
is_cp_special(int cp_index)
{
	cp_index &= ~SYSTEM_CHARSET_FLAG;
	return codepages[cp_index].table == table_utf_8;
}
