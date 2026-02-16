#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <libstemmer.h>

int main(int argc, char *argv[]) {
    // if no arguments provided
    if (argc == 1) {
        printf("You should provide at least one argumnet!\n"
               "Here are a few examples, of how you should use this command\n"
               "ask How to see contant of a text file?\n"
               "ask How to update my system?\n");
        return 1;
    }

    FILE *commands_file = fopen("commands.json", "r");
    if (commands_file == NULL) {
        printf("Error opening file!\n"
               "(commands_file pointer is equals to null)\n"
               "Check is there actually a file called \"commands.json\" in this directory\n");
        return 1;
    }

    // get file lenth
    fseek(commands_file, 0, SEEK_END);
    long lenth = ftell(commands_file);
    fseek(commands_file, 0, SEEK_SET);

    // load commands.json file in memory
    char *json_string = malloc(lenth + 1);
    fread(json_string, 1, lenth, commands_file);
    json_string[lenth] = '\0';
    fclose(commands_file);

    // parse json
    cJSON *json = cJSON_Parse(json_string);
    free(json_string);

    if (json == NULL) {
        printf("JSON parsing error!\n"
               "(json pointer is equals to null)\n"
               "There must be an error in the \"commands.json file\"");
        return 1;
    }

    // chech, if the *json is an array
    if (!cJSON_IsArray(json)) {
        printf("JSON parsing error!\n"
               "(expexted *json to be array)\n"
               "There must be an error in the \"commands.json file\"");
        cJSON_Delete(json);
        return 1;
    }

    // init stemmer
    struct sb_stemmer *stemmer = sb_stemmer_new("english", "UTF_8");
    if (stemmer == NULL) {
        printf("Can't create a stemmer structure!\n"
               "(pointer stemmer is equals to null)\n");
        cJSON_Delete(json);
        return 1;
    }

    // variables
    cJSON *command = NULL;
    cJSON *command_name = NULL;
    cJSON *command_tags = NULL;
    cJSON *best_candidate = NULL;
    cJSON *tag = NULL;

    int tag_index;
    int best_score = 0;
    int current_score;
    int commands_index = 0;
    int max_len = 0;

    int *tag_frequency = malloc(sizeof(int) * (argc - 1));
    char **stemmed_args = malloc(sizeof(char*) * (argc - 1));

    // stemm arguments and convert to lowercase
    for (int i = 1; i < argc; i++) {
        char lowercase[256];

        if (strlen(argv[i]) > 255) {
            printf("Too long argument: %s\n", argv[i]);
            return 1;
        }

        int len = strlen(argv[i]);
        for (int j = 0; j < len; j++) {
            lowercase[j] = tolower(argv[i][j]);
        }
        lowercase[len] = 0;
        const unsigned char *stemmed = sb_stemmer_stem(
                    stemmer,
                    (const unsigned char*)lowercase,
                    strlen(lowercase)
                );
        if (stemmed == NULL) {
            printf("Fatal error: stemmer returned null!\n");
            return 1;
        }
        printf("STEMMED: %s\n", stemmed);
        stemmed_args[i - 1] = malloc(sizeof(char) * (strlen(stemmed) + 1));
        strcpy(stemmed_args[i - 1], stemmed);
    }

    // search loop
    while (true) {
        // reset current score
        current_score = 0;

        // get command
        command = cJSON_GetArrayItem(json, commands_index);
        command_name = cJSON_GetObjectItem(command, "name");
        command_tags = cJSON_GetObjectItem(command, "tags");

        // get the score of the command
        tag_index = 0;
        tag = cJSON_GetArrayItem(command_tags, tag_index);

        while (true) {
            if (!cJSON_IsString(tag)) {
                    printf("Tag is not a string (bruh)\n");
                    return 1;
                }
            char tag_stem[256];

            const unsigned char *stemmed_tag = sb_stemmer_stem(
                    stemmer,
                    (const unsigned char*)tag->valuestring,
                    strlen(tag->valuestring)
                );
            strcpy(tag_stem, stemmed_tag);

            for (int i = 1; i < argc; i++) {
                if (strcmp(tag_stem, stemmed_args[i - 1]) == 0)
                    current_score++;
            }

            if (tag->next == NULL)
                break;

            tag_index++;
            tag = cJSON_GetArrayItem(command_tags, tag_index);
        }


        if (current_score > best_score) {
            best_score = current_score;
            best_candidate = command;
        }

        // if there are no more commands
        if (command->next == NULL)
            break;

        // update the value of *command var
        commands_index++;
    }

    // free
    free(stemmed_args);
    free(tag_frequency);

    printf("Best score: %d\n", best_score);
    if (best_candidate != NULL)
        printf("Best candidate: %s\n", cJSON_GetObjectItem(best_candidate, "name")->valuestring);
    else
        printf("Cannot find anything...\n");

    // end of the program
    sb_stemmer_delete(stemmer);
    cJSON_Delete(json);
    return 0;
}