#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <libstemmer.h>
#include <math.h>

int main(int argc, char *argv[]) {
    // if no arguments provided
    if (argc == 1) {
        printf("You should provide at least one argumnet!\n"
               "Here are a few examples, of how you should use this command\n"
               "ask How to see content of a text file?\n"
               "ask How to create a directory?\n");
        return 1;
    }

    FILE *commands_file = fopen("commands.json", "r");
    if (commands_file == NULL) {
        printf("Error opening file!\n"
               "(commands_file pointer is equals to null)\n"
               "Is there actually a file called \"commands.json\"?\n");
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

    // check, if the *json is an array
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

    cJSON *best_candidate = NULL;
    cJSON *tag = NULL;

    size_t json_size = cJSON_GetArraySize(json); // soon, a lot of loops will use this
    int tag_index;

    int *tag_frequency = calloc((argc - 1), sizeof(int));
    bool *scores = calloc(cJSON_GetArraySize(json) * (argc - 1), sizeof(bool));
    double tf_idf_score = 0;
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

        // stemm this thing
        const unsigned char *stemmed = sb_stemmer_stem(
                    stemmer,
                    (const unsigned char*)lowercase,
                    strlen(lowercase)
                );
        
        if (stemmed == NULL) {
            printf("Fatal error: stemmer returned null!\n");
            return 1;
        }

        stemmed_args[i - 1] = malloc(sizeof(char) * (strlen(stemmed) + 1));
        strcpy(stemmed_args[i - 1], stemmed);
    }

    // variables required for search loop
    int commands_index = 0;
    cJSON *command = NULL;
    cJSON *command_name = NULL;
    cJSON *command_tags = NULL;

    // search loop
    while (true) {

        // get command
        command = cJSON_GetArrayItem(json, commands_index);
        command_name = cJSON_GetObjectItem(command, "name");
        command_tags = cJSON_GetObjectItem(command, "tags");

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
                if (strcmp(tag_stem, stemmed_args[i - 1]) == 0) {
                    tag_frequency[i - 1]++;
                    scores[commands_index * (argc - 1) + (i - 1)] = true;
                    printf("Find a match!!!\n");
                }
            }

            if (tag->next == NULL)
                break;

            tag_index++;
            tag = cJSON_GetArrayItem(command_tags, tag_index);
        }

        // if there are no more commands
        if (command->next == NULL)
            break;

        // update the value of *command var
        commands_index++;
    }

    // tf-idf implementation
    int best_candidate_index = -1;
    double current_score;
    double best_score = -1;
    for (int i = 0; i < json_size; i++) {

        current_score = 0;
        for (int j = 0; j < (argc - 1); j++) {
            printf("score[i * (argc - 1) + j]: %d\n", (int)scores[i * (argc - 1) + j]);
            current_score += (double)(scores[i * (argc - 1) + j]) * log((json_size / (tag_frequency[j] + 1)));
        }
        printf("Score: %lf\n", current_score);

        if (current_score > best_score) {
            best_score = current_score;
            best_candidate_index = i;
        }
    }

    best_candidate = cJSON_GetArrayItem(json, best_candidate_index);
    if (best_candidate_index == -1) {
        printf("Best candidate is equals to null!\n");
        printf("Best candidate index: %d\n", best_candidate_index);
        return 1;
    }
    printf("Best score: %lf\n", best_score);
    printf("Best candidate: %s\n", cJSON_GetObjectItem(best_candidate, "name")->valuestring); // error here

    // free
    free(stemmed_args);
    free(tag_frequency);
    free(scores);

    // end of the program
    sb_stemmer_delete(stemmer);
    cJSON_Delete(json);
    return 0;
}