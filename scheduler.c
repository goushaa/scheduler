#include "headers.h"


int main(int argc, char * argv[])
{
    //initClk();
    if(argc != 2)
    {
        printf("sad ya5oya\n");
        FILE *output_file = fopen("output.txt", "w");
        if (output_file == NULL) {
            perror("Error opening file");
            return 1;
        }

        fprintf(output_file, "saaaaad\n");
        exit(-1);
    }
    int algorithm = atoi(argv[0]);
    int quantum = atoi(argv[1]);
    printf("ana aho w ma3aya\n");
    printf("%d\n",algorithm);
    printf("%d\n",quantum);

    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    fprintf(output_file, "algorithm = %d\n", algorithm);
    fprintf(output_file, "quantum = %d\n", quantum);
    

    // close the file
    fclose(output_file);
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    
    //destroyClk(true);
}
