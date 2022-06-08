#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// NOTE: why is windows like this
#ifdef PLATFORM_WIN32
  #define strtok_r strtok_s
#endif

enum token_type_e
{
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_INTEGER,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_VEC2I,
    TOKEN_TYPE_COUNT,
};

struct token_node_t
{
    token_type_e type;
    char name[20];
    union {
        char  string[20];
        int   number_i;
        float number_f;
        int   vec2i[2];
    };
    token_node_t* next;
};

#define MAGIC_NUMBER  "ENT_ASSET\n"
#define MAGIC_NR_SIZE 10
token_node_t* parse_asset_file(const char* filename)
{
    // open asset file
    FILE* asset_file = fopen(filename, "r");

    // check the magic number
    char* magic_nr_buf = (char*) malloc(MAGIC_NR_SIZE);
    fread(magic_nr_buf, sizeof(char), MAGIC_NR_SIZE, asset_file);
    magic_nr_buf[MAGIC_NR_SIZE] = '\0';
    if (strcmp(magic_nr_buf, MAGIC_NUMBER)) exit(-1);

    fseek(asset_file, 0L, SEEK_END);
    const size_t file_size = ftell(asset_file) - MAGIC_NR_SIZE;
    fseek(asset_file, 0L, SEEK_SET);
    fseek(asset_file, MAGIC_NR_SIZE, SEEK_SET);

    // read in file buffer
    char* asset_file_buf = (char*) malloc(file_size+1);
    fread(asset_file_buf, sizeof(char), file_size, asset_file);
    asset_file_buf[file_size] = '\0';
    fclose(asset_file);

    token_node_t* head = (token_node_t*) malloc(sizeof(token_node_t));
    token_node_t* node = head;
    node->next = nullptr;
    char* saveptr = NULL;
    char* statement_token = strtok_r(asset_file_buf, "\n", &saveptr);
    while (statement_token != NULL)
    {
        //printf("STATEMENT: %s\n", statement_token);

        char* node_name  = strtok(statement_token, " = ");
        strcpy(node->name, node_name);

        // detect type
        char* node_value = strtok(NULL, " = ");
        if (node_value[0] == '"' && node_value[strlen(node_value)-1] == '"')
        {
            node->type    = TOKEN_TYPE_STRING;
            node_value[strlen(node_value) - 1] = '\0';
            strncpy(node->string, &node_value[1], strlen(node_value));
        }
        else if (node_value[0] == '{' && node_value[strlen(node_value)-1] == '}')
        {
            node->type    = TOKEN_TYPE_VEC2I;
            node_value[0] = ' ';
            node_value[strlen(node_value)-1] = ' ';
            node->vec2i[0] = atoi(strtok(node_value, ","));
            node->vec2i[1] = atoi(strtok(NULL, ","));
        }
        else if (node_value[strlen(node_value)-1] == 'f')
        {
            node->type    = TOKEN_TYPE_FLOAT;
            node->number_f = atof(node_value);
        }
        else
        {
            node->type    = TOKEN_TYPE_INTEGER;
            node->number_i = atoi(node_value);
        }

        node->next = (token_node_t*) malloc(sizeof(token_node_t));
        node = node->next;
        node->next = nullptr;

        statement_token = strtok_r(NULL, "\n", &saveptr);
    }

    return head;
}

Entity create_entity_from_file(const char* asset_file_name, platform_api_t* platform, platform_window_t* window)
{
    const char* res_folder = "res/";
    char* filepath = (char*) malloc(sizeof(char) * (strlen(res_folder) + strlen(asset_file_name)));
    strcpy(filepath, res_folder);
    strcpy(&filepath[strlen(res_folder)], asset_file_name);

    token_node_t* head = parse_asset_file(filepath);
    token_node_t* node;

    Entity entity = {};
    entity.active = true;
    entity.freed  = false;

    // TODO only set these for the right entity
    entity.state  = ENT_STATE_MOVE;
    entity.orient = ENT_ORIENT_DOWN;
    entity.flags |= ENT_FLAG_IS_COLLIDER;
    entity.flags |= ENT_FLAG_CMD_CONTROLLED;
    entity.flags |= ENT_FLAG_IS_REWINDABLE;
    entity.flags |= ENT_FLAG_IS_ANIMATED;
    Rewind::initializeFrames(entity);
    command_init(entity);

    u32 entity_width  = 0;
    u32 entity_height = 0;
    u32 tilewidth     = 0;
    u32 tileheight    = 0;
    rect_t spritebox  = {0,0,0,0};

    entity.sprite.pivot = {0.5f, 0.5f}; // TODO hardcoded

    for (node = head; node->next != nullptr; node = node->next)
    {
        switch (node->type)
        {
            case TOKEN_TYPE_STRING:
            {
                if (!strcmp(node->name, "type"))
                {
                    strcpy(entity.type, node->string);
                }
                else if (!strcmp(node->name, "spritesheet"))
                {
                    entity.sprite.tex   = resourcemgr_texture_load(node->string, platform, window);
                }
            } break;
            case TOKEN_TYPE_INTEGER:
            {
                if (!strcmp(node->name, "width"))
                {
                    entity_width  = node->number_i;
                }
                else if (!strcmp(node->name, "height"))
                {
                    entity_height = node->number_i;
                }
                else if (!strcmp(node->name, "tilewidth"))
                {
                    tilewidth = node->number_i;
                }
                else if (!strcmp(node->name, "tileheight"))
                {
                    tileheight = node->number_i;
                }
            } break;
            case TOKEN_TYPE_FLOAT:
            {
                //printf("PARSER: FLOAT VALUE UNUSED\n");
            } break;
            case TOKEN_TYPE_VEC2I:
            {
                if (!strcmp(node->name, "sprite"))
                {
                    spritebox.left = node->vec2i[0];
                    spritebox.top = node->vec2i[1];
                }
            } break;
            default:
            {
                UNREACHABLE("Token type '%u' not implemented", node->type);
            } break;
        }
    }

    // TODO hardcoded
    if (strcmp(entity.type, "skeleton") == 0)
    {
        entity.anims[ENT_ORIENT_UP    + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 0, 0 +  0, 16};
        entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 0, 0 + 32, 16};
        entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 0, 0 + 64, 16};
        entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 0, 0 + 96, 16};

        entity.anims[ENT_ORIENT_UP    + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 0, 128 +  0, 16};
        entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 0, 128 + 32, 16};
        entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 0, 128 + 64, 16};
        entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 0, 128 + 96, 16};

        //entity.anims[ENT_ORIENT_UP    + (ENT_STATE_PICKUP * ENT_ORIENT_COUNT)] = {4, 0, 64, 16};
        //entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_PICKUP * ENT_ORIENT_COUNT)] = {4, 0, 0,  16};
        //entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_PICKUP * ENT_ORIENT_COUNT)] = {4, 0, 96, 16};
        //entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_PICKUP * ENT_ORIENT_COUNT)] = {4, 0, 32, 16};

        entity.anims[ENT_ORIENT_UP    + (ENT_STATE_HOLD * ENT_ORIENT_COUNT)] = {4, 0, 384 +  0, 16};
        entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_HOLD * ENT_ORIENT_COUNT)] = {4, 0, 384 + 32, 16};
        entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_HOLD * ENT_ORIENT_COUNT)] = {4, 0, 384 + 64, 16};
        entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_HOLD * ENT_ORIENT_COUNT)] = {4, 0, 384 + 96, 16};

    }
    else if (strcmp(entity.type, "necromancer") == 0)
    {
        entity.anims[ENT_ORIENT_UP    + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 64, 0 +  0, 16};
        entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 64, 0 + 32, 16};
        entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 64, 0 + 64, 16};
        entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_MOVE * ENT_ORIENT_COUNT)] = {4, 64, 0 + 96, 16};

        entity.anims[ENT_ORIENT_UP    + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 64, 128 +  0, 16};
        entity.anims[ENT_ORIENT_DOWN  + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 64, 128 + 32, 16};
        entity.anims[ENT_ORIENT_LEFT  + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 64, 128 + 64, 16};
        entity.anims[ENT_ORIENT_RIGHT + (ENT_STATE_ATTACK * ENT_ORIENT_COUNT)] = {4, 64, 128 + 96, 16};
    }

    spritebox.left   *= tilewidth;
    spritebox.top    *= tileheight;
    spritebox.w       = entity_width  * tilewidth;
    spritebox.h       = entity_height * tileheight;
    entity.collider   = spritebox;
    entity.sprite.box = spritebox;

    // cleanup
    free(filepath);
    node = head;
    while (node != nullptr) // free linked list of tokens
    {
        token_node_t* node_to_free = node;
        node = node->next;
        free(node_to_free);
    }

    return entity;
}

#if 0
int main()
{
    token_node_t head = parse_asset_file("skeleton.ent");
    token_node_t* node;

    // traverse linked list of tokens
    printf("\n");
    printf("TOKENS: \n");
    for (node = &head; node->next != nullptr; node = node->next)
    {
        switch (node->type)
        {
            case TOKEN_TYPE_STRING:
            {
                printf("STRING: ");
                printf("%s ", node->name);
                printf("%s\n", node->string);
            } break;
            case TOKEN_TYPE_INTEGER:
            {
                printf("INTEGER: ");
                printf("%s ", node->name);
                printf("%i\n", node->number_i);
            } break;
            case TOKEN_TYPE_FLOAT:
            {
                printf("FLOAT: ");
                printf("%s ", node->name);
                printf("%f\n", node->number_f);
            } break;
            case TOKEN_TYPE_VEC2I:
            {
                printf("VEC2I: ");
                printf("%s ", node->name);
                printf("%i, %i\n", node->vec2i[0], node->vec2i[1]);
            } break;
            default:
            {
                // TODO unreachable
                exit(-1);
            }
        }
    }
}
#endif
