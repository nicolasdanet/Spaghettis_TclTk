
/* < https://opensource.org/licenses/BSD-3-Clause > */

// ====================================

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static const char *test_BorisVian = "Le sentier longeait la falaise."
    " Il \xc3\xa9tait bord\xc3\xa9 de calamines en fleur et de brouillouses un peu pass\xc3\xa9"
    "es dont les p\xc3\xa9tales noircis jonchaient le sol. Des insectes pointus avaient creus\xc3\xa9"
    " le sol de mille petits trous ; sous les pieds, c\xe2\x80\x99\xc3\xa9tait comme "
    "de l\xe2\x80\x99\xc3\xa9ponge morte de froid.";

static const char *test_ArnoSchmidt = "Mein Leben ? ! : ist kein Kontinuum!"
    " (nicht blo\xc3\x9f Tag und Nacht in wei\xc3\x9f und schwarze St\xc3\xbc""cke zerbroche"
    "n ! Denn auch am Tage ist bei mir der ein Anderer, der zur Bahn geht; im Amt sitzt; b\xc3\xbc"
    "chert; durch Haine stelzt; begattet; schwatzt; schreibt;"
    " Tausendsdenker; auseinanderfallender F\xc3\xa4"
    "cher; der rennt; raucht; kotet; radioh\xc3\xb6rt; \'Herr Landrat\' sagt: that\'s me!)"
    " ein Tablett voll glitzender snapshots.";

static const char *test_UTF8 =
    "\xc3\x84" "a" "\xc3\xa4" "b"
    "\xc3\x9c" "c" "\xc3\xbc" "d"
    "\xc3\x9f" "e" "\xd0\xaf" "f"
    "\xd0\x91" "g" "\xd0\x93" "h"
    "\xd0\x94" "i" "\xd0\x96" "j"
    "\xd0\x99" "k" "\xc5\x81" "l"
    "\xc4\x84" "m" "\xc5\xbb" "n"
    "\xc4\x98" "o" "\xc4\x86" "p";

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
void test30__utf8() {
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

TTT_BEGIN (UTF8, 30, "UTF8 - Basic")

    int i, j = 0;
    uint16_t w1[1024] = { 0 };
    uint16_t w2[1024] = { 0 };
    char u1[1024] = { 0 };
    char u2[1024] = { 0 };
    
    int size1 = u8_utf8toucs2 (w1, 1024, (char *)test_BorisVian, -1);
    int size2 = u8_utf8toucs2 (w2, 1024, (char *)test_ArnoSchmidt, -1);
    
    TTT_EXPECT (size1 == 271);
    TTT_EXPECT (size2 == 396);
    
    TTT_EXPECT (u8_ucs2toutf8 (u1, 1024, w1, -1) != -1);
    TTT_EXPECT (u8_ucs2toutf8 (u2, 1024, w2, -1) != -1);

    TTT_EXPECT (strcmp (test_BorisVian, u1) == 0);
    TTT_EXPECT (strcmp (test_ArnoSchmidt, u2) == 0);
    
    for (i = 0; i < 32; i++) {
        TTT_EXPECT (u8_charnum ((char *)test_UTF8, u8_offset ((char *)test_UTF8, i)) == i);
    }
    
    for (i = 0; i < 16; i++) {
        u8_inc ((char *)test_UTF8, &j); u8_inc ((char *)test_UTF8, &j); TTT_EXPECT ((j % 3) == 0);
    }
    
    for (i = 0; i < 16; i++) {
        u8_dec ((char *)test_UTF8, &j); u8_dec ((char *)test_UTF8, &j); TTT_EXPECT ((j % 3) == 0);
    }
    
TTT_END

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
}
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
