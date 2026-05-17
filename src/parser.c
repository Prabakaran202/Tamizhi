#include "parser.h"
#include "codegen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// மெயின் பிளாக் ஜெனரேட் ஆகிவிட்டதா என்பதை அறியும் ஃப்ளாக் (Flag)
int main_generated = 0;

// டோக்கன் வேல்யூ காலியாக இல்லாமல் சரியாக உள்ளதா என சரிபார்க்கும் பங்க்ஷன்
int is_valid(Token t) {
    if (strlen(t.value) == 0) return 0;
    return 1;
}

// செமிகோலன் (Anatomy: ;) வரும் வரை இடையில் இருக்கும் தேவையற்ற டோக்கன்களை தவிர்க்க
void skip_to_semicolon(FILE *file) {
    Token t;
    while ((t = get_next_token(file)).type != 21 && t.type != T_EOF);
}

void parse_statement(FILE *file, Token t);

// 🌟 ஹெடர் பிரீ-ஸ்கேன்: பங்க்ஷன் பாடிக்கு வெளியே செமிகோலன் ';' இருந்தாலும் அதைத் தவிர்த்து பெயர்களை மட்டும் ரெஜிஸ்டர் செய்யும்
void scan_headers(FILE *file) {
    Token t;
    rewind(file); // ஃபைலின் தொடக்கத்திற்கு பாயிண்டரை கொண்டு செல்கிறோம்
    fprintf(stderr, " -> Starting Header Pre-Scan...\n");

    while ((t = get_next_token(file)).type != T_EOF) {
        // 'fun' அல்லது 'நிகழ்' வார்த்தையைப் பார்த்தால் மட்டுமே உள்ளே நுழையும்
        if (strcmp(t.value, "fun") == 0 || t.type == T_FUNC || strcmp(t.value, "நிகழ்") == 0) {
            Token name = get_next_token(file);

            if (is_valid(name)) {
                // 'add()' என ஒட்டி வந்தால் பிராக்கெட்டைத் துண்டித்து பெயரை மட்டும் பிரிக்கிறது
                char *bracket_ptr = strchr(name.value, '(');
                if (bracket_ptr != NULL) {
                    *bracket_ptr = '\0';
                }

                // மெயின் பிளாக்கைத் தவிர்த்து மற்றவற்றை குளோபல் பங்க்ஷனாக லாக் செய்கிறது
                if (strcmp(name.value, "main") != 0 && strcmp(name.value, "முதன்மை") != 0) {
                    fprintf(stderr, "    [Header] Registered: %s\n", name.value);
                }

                // பங்க்ஷனோட மொத்த பாடி பிளாக்கையும் ({ லிருந்து }) ஸ்கிப் பண்ணிக் கடக்கிறது
                Token skip_t;
                while ((skip_t = get_next_token(file)).type != T_EOF) {
                    if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) {
                        int scan_brace_count = 1;
                        while ((skip_t = get_next_token(file)).type != T_EOF) {
                            if (skip_t.type == 22 || strcmp(skip_t.value, "{") == 0) scan_brace_count++;
                            if (skip_t.type == 23 || strcmp(skip_t.value, "}") == 0) {
                                scan_brace_count--;
                                if (skip_t.type == 23 || scan_brace_count <= 0) break; // பாடி பிளாக் முடிந்தது
                            }
                        }
                        break;
                    }
                    // பிராக்கெட் ஓபன் ஆவதற்குள் அடுத்த 'fun' வார்த்தை வந்தால் லூப் உடையாமல் இருக்க ரீசெட் லாஜிக்
                    if (strcmp(skip_t.value, "fun") == 0 || strcmp(skip_t.value, "நிகழ்") == 0) {
                        long back_pos = ftell(file);
                        fseek(file, back_pos - (long)strlen(skip_t.value), SEEK_SET);
                        break;
                    }
                }
            }
        }
        // 🌟 உங்க ஐடியா: ஒருவேளை பங்க்ஷனுக்கு வெளியே செமிகோலன் ';' இருந்தால், இந்த லூப் அதை தானாகவே கடந்து அடுத்த 'fun' தேடிச் செல்லும்!
    }
    rewind(file); // பிரீ-ஸ்கேன் முடிந்து மெயின் எக்ஸிகியூஷன் பாடி படிக்க ரீவைண்ட் செய்யப்படுகிறது
}

// 🌟 மெயின் பார்ஸர் இன்ஜின்: மெயின் மற்றும் பூட்டர் பிளாக்குகளின் பொசிஷனைக் கண்டறிந்து இயக்கும்
void parse(FILE *file) {
    Token t;
    long main_pos = -1L;   // மெயின் பாடி தொடங்கும் புள்ளி
    long footer_pos = -1L; // பூட்டர் பாடி தொடங்கும் புள்ளி
    fprintf(stderr, "\n[Parser] --- Tamizhi Engine: Universal Analysis Started ---\n");

    // Phase 1: குளோபல் பங்க்ஷன்களை மட்டும் முன்கூட்டியே ஸ்கேன் செய்து ரிஜிஸ்டர் செய்தல்
    scan_headers(file);

    // Phase 2: மெயின் மற்றும் பூட்டர் பிளாக்குகள் எங்குள்ளது என்று பொசிஷனை மட்டும் லாக் செய்தல்
    rewind(file);
    while ((t = get_next_token(file)).type != T_EOF) {
        if (strcmp(t.value, "முதன்மை") == 0 || strcmp(t.value, "main") == 0) {
            Token brace_t = get_next_token(file);
            if (brace_t.type != 22 && strcmp(brace_t.value, "{") != 0) {
                // '{' பிராக்கெட் வரும் வரை டோக்கன்களைத் தேடி முன்னோக்கி நகர்கிறது
                while ((brace_t = get_next_token(file)).type != 22 && brace_t.type != T_EOF);
            }
            main_pos = ftell(file); // மெயின் பாடியின் தொடக்கப் புள்ளி லாக் செய்யப்பட்டது
        }
        else if (strcmp(t.value, "பூட்டர்") == 0 || strcmp(t.value, "footer") == 0) {
            Token brace_t = get_next_token(file);
            if (brace_t.type != 22 && strcmp(brace_t.value, "{") != 0) {
                while ((brace_t = get_next_token(file)).type != 22 && brace_t.type != T_EOF);
            }
            footer_pos = ftell(file); // பூட்டர் பாடியின் தொடக்கப் புள்ளி லாக் செய்யப்பட்டது
        }
    }

    // Phase 3: மெயின் (Main) பகுதிக்குள் இருப்பவற்றை இயக்குதல்
    fprintf(stderr, " -> Phase 2 [Body]: Mapping logic to DNA-VM...\n");
    if (!main_generated) {
        tamizhi_generate_entry(); // எல்எல்விஎம் (LLVM) சிஸ்டம்கான எண்ட்ரி பாயிண்ட் பில்ட் ஆகிறது
        main_generated = 1;
    }

    if (main_pos != -1L) {
        clearerr(file); // முந்தைய லூப்பால் ஏற்பட்ட எண்ட்-ஆஃப்-ஃபைல் (EOF) ஸ்டேட்டை சுத்தப்படுத்துகிறது
        fseek(file, main_pos, SEEK_SET);
        int main_brace_count = 1;

        while (main_brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
            if (t.type == 22 || strcmp(t.value, "{") == 0) {
                main_brace_count++;
            }
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                main_brace_count--;
                if (main_brace_count <= 0) break; 
            }
            parse_statement(file, t); // மெயின் பாடிக்குள் இருக்கும் வரிகள் ஒவ்வொன்றாக இயங்கும்
        }
    }

    // Phase 4: பூட்டர் (Footer) பகுதிக்குத் தாவி இயக்குதல்
    if (footer_pos != -1L) {
        fprintf(stderr, " -> Phase 3 [Footer]: Launching execution...\n");
        clearerr(file); // ஸ்ட்ரீம் எரர் ஸ்டேட்டை மீண்டும் ரீசெட் செய்கிறது
        fseek(file, footer_pos, SEEK_SET);

        int footer_brace_count = 1;
        while (footer_brace_count > 0 && (t = get_next_token(file)).type != T_EOF) {
            if (t.type == 22 || strcmp(t.value, "{") == 0) footer_brace_count++;
            if (t.type == 23 || strcmp(t.value, "}") == 0) {
                footer_brace_count--;
                if (footer_brace_count <= 0) break; // பூட்டர் பாடி முடிந்தது!
            }
            parse_statement(file, t); // பூட்டர் பிளாக்குக்குள் இருக்கும் ஃபங்ஷன் கால்கள் இயங்கும்
        }
        // 🌟 உங்க ஐடியா: பூட்டர் க்ளோசிங் பிராக்கெட்டுக்கு வெளியே செமிகோலன் ';' இருந்தாலும், 
        // லூப் ஏற்கனவே பிரேக் ஆகிவிட்டதால் இன்ஜின் அதை ரீட் பண்ணாமல் பாதுகாப்பாக நிறைவடையும்!
    }

    fprintf(stderr, "[Parser] --- Analysis Completed Successfully ---\n\n");
}

// 🌟 ஸ்டேட்மென்ட் அனலைசர்: ஒவ்வொரு தனித்தனி வரிகளையும் (Num, Str, ID, Print) பகுப்பாய்வு செய்யும் இடம்
void parse_statement(FILE *file, Token t) {
    if (!is_valid(t)) return;

    if (t.type == 21 || strcmp(t.value, ";") == 0) return;

    // 1. எண்கள் பிளாக் (Num a = 10 ;)
    if (t.type == T_INT || strcmp(t.value, "Num") == 0 || strcmp(t.value, "எண்") == 0) {
        Token name_token = get_next_token(file); 
        Token next = get_next_token(file);
        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file); 
        }
        Token val_token = get_next_token(file);
        if (isdigit(val_token.value[0])) {
            tamizhi_gen_var(name_token.value, atoi(val_token.value));
        }
    }
    // 2. சரங்கள் பிளாக் (Str s = "Hello" ;)
    else if (t.type == T_STR || strcmp(t.value, "Str") == 0 || strcmp(t.value, "வரி") == 0) {
        Token name_token = get_next_token(file);
        Token next = get_next_token(file);
        while (next.type != 20 && next.type != T_EOF) {
            next = get_next_token(file);
        }
        Token val_token = get_next_token(file);
        if (is_valid(val_token)) {
            tamizhi_gen_str(name_token.value, val_token.value);
        }
    }
    // 3. ஐடென்டிஃபையர் பிளாக்: வேரியபிள் அப்டேட் அல்லது பங்க்ஷன் கால்
    else if (t.type == T_ID) {
        char var_name[50];
        strcpy(var_name, t.value); 
        long current_pos = ftell(file);
        Token next_t = get_next_token(file);

        if (next_t.type == 20) { // '=' -> வேரியபிள் அப்டேட் லாஜிக்
            Token v1 = get_next_token(file);
            Token op = get_next_token(file);
            if (op.type == 19) { // '+'
                Token v2 = get_next_token(file);
                tamizhi_gen_var_add(var_name, v1.value, v2.value);
            }
        } 
        else if (next_t.type == 15 || strcmp(next_t.value, "(") == 0) { // '(' -> பங்க்ஷன் கால் லாஜிக்
            // 🌟 நடப்பு பங்க்ஷன் கால் வரியின் செமிகோலன் (;) வரை டோக்கன்களைப் படித்து முடிக்கிறது
            Token tmp;
            while ((tmp = get_next_token(file)).type != T_EOF) {
                if (tmp.type == 21 || strcmp(tmp.value, ";") == 0) {
                    break;
                }
            }
            long post_call_pos = ftell(file); // அடுத்த பங்க்ஷன் காலுக்கான வரியை லாக் செய்கிறோம்

            // குளோபல் ஸ்கோப்பில் பங்க்ஷன் பாடியைத் தேடிப் பிடிக்கிறது
            clearerr(file);
            rewind(file);
            Token find_f;
            long func_body_pos = -1L;

            while ((find_f = get_next_token(file)).type != T_EOF) {
                if (is_valid(find_f) && (strcmp(find_f.value, "fun") == 0 || find_f.type == T_FUNC || strcmp(find_f.value, "நிகழ்") == 0)) {
                    Token name = get_next_token(file);
                    if (is_valid(name) && strcmp(name.value, var_name) == 0) {
                        while ((find_f = get_next_token(file)).type != 22 && find_f.type != T_EOF); // '{' தேடுகிறது
                        func_body_pos = ftell(file); 
                        break;
                    }
                }
            }

            // 🌟 லீனியர் ஃப்ளோ கன்ட்ரோல்: பங்க்ஷன் பாடிக்குள் இருக்கும் வரிகளை செமிகோலன் வரிசையாக இயக்குகிறோம்
            if (func_body_pos != -1L) {
                clearerr(file);
                fseek(file, func_body_pos, SEEK_SET);
                int body_brace_count = 1;
                Token body_t;

                while (body_brace_count > 0 && (body_t = get_next_token(file)).type != T_EOF) {
                    if (body_t.type == 22 || strcmp(body_t.value, "{") == 0) {
                        body_brace_count++;
                    }
                    if (body_t.type == 23 || strcmp(body_t.value, "}") == 0) {
                        body_brace_count--;
                        if (body_brace_count <= 0) break; // ஃபங்ஷன் பாடி முடிந்தது! அடுத்த ஃபங்ஷனுக்குப் போகலாம்!
                    }
                    
                    // செமிகோலன் அல்லது பிராக்கெட் இல்லை எனில், ஸ்டேட்மென்ட்டின் தொடக்கத்தை அனுப்பி வரியை முழுமையாக படிக்க வைக்கிறது
                    if (body_t.type != 21 && strcmp(body_t.value, ";") != 0 && body_t.type != 22 && body_t.type != 23) {
                        parse_statement(file, body_t);
                        
                        // வரியின் முடிவு செமிகோலன் வரை உள்-லூப் படித்த பிறகு, பாயிண்டரை அடுத்த வரியின் தொடக்கத்தில் நிறுத்துகிறது!
                        func_body_pos = ftell(file);
                        fseek(file, func_body_pos, SEEK_SET);
                    }
                }
            }
            
            // பங்க்ஷன் பாடி வெற்றிகரமாக முடிந்ததும், பழைய பூட்டர் காலிற்குப் பிறகு இருக்கும் அடுத்த பங்க்ஷன் காலுக்கு பாயிண்டர் திரும்புகிறது
            clearerr(file);
            fseek(file, post_call_pos, SEEK_SET); 
        } else {
            fseek(file, current_pos, SEEK_SET);
        }
    }
    // 4. லூப் பிளாக் (for / சு)
    else if (t.type == T_FOR || strcmp(t.value, "சு") == 0) {
        Token limit_token = get_next_token(file); 
        int limit = 0;

        if (isdigit(limit_token.value[0])) {
            limit = atoi(limit_token.value);
        } else {
            limit = 3; 
        }

        Token next = get_next_token(file);
        if (next.type == 22) {
            tamizhi_gen_loop_start(limit);
            Token body_t;
            while ((body_t = get_next_token(file)).type != 23 && body_t.type != T_EOF) {
                parse_statement(file, body_t);
            }
            tamizhi_gen_loop_end();
        }
    }
    // 5. அச்சிடு பிளாக் (print ;)
    else if (t.type == T_PRINT || strcmp(t.value, "அச்சிடு") == 0) {
        Token first = get_next_token(file);
        if (first.type == 15) first = get_next_token(file); 
        tamizhi_gen_print(first.value);
    }
}
