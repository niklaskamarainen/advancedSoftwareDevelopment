#include <stdio.h>
#include <stdlib.h>

struct Student{
    int id;
    int grade;
};

void task1(void){
    int temperatures[5];
    

    for(int i = 0; i < 5; i++){
        printf("Anna lampotila %d: ", i + 1);

        if(scanf("%d", &temperatures[i]) != 1){
            printf("Virheellinen syote.\n");
            
            return;
        }
    }

    int sum = 0;
    int min = temperatures[0];
    int max = temperatures[0];

    printf("\nSyotetyt lampotilat:\n");

    for(int i = 0; i < 5; i++){
        printf("%d\n", temperatures[i]);
        
        sum += temperatures[i];

        if(temperatures[i] < min){
            min = temperatures[i];
        }

        if(temperatures[i] > max){
            max = temperatures[i];
        }
    }

    double average = (double)sum / 5;
    

    printf("Pienin lampotila: %d\n", min);
    printf("Suurin lampotila: %d\n", max);
    printf("Keskiarvo: %.2f\n", average);

    return;
}

int find_min(int array[], int size){
    int min = array[0];

    for(int i = 1; i < size; i++){
        if(array[i] < min){
            min = array[i];
        }
    }

    return min;
}

int find_max(int array[], int size){
    int max = array[0];

    for(int i = 1; i < size; i++){
        if(array[i] > max){
            max = array[i];
        }
    }

    return max;
}

double find_average(int array[], int size){
    int sum = 0;

    for(int i = 0; i < size; i++){
        sum += array[i]; 
    }
    
    double average = (double)sum / size;

    return average;
}

int count_even(int array[], int size){
    int count = 0;

    for(int i = 0; i < size; i++){
        if(array[i] % 2 == 0){
            count++;
        }
    }

    return count;
}

void task2(void){
    int data[] = {4, 8, 1, 9, 2, 7, 3};
    int size = 7;

    int min = find_min(data, size);
    int max = find_max(data, size);
    double average = find_average(data, size);
    int even_count = count_even(data, size);

    printf("Pienin arvo: %d\n", min);
    printf("Suurin arvo: %d\n", max);
    printf("Keskiarvo: %.2f\n", average);
    printf("Parillisten maara: %d\n", even_count);
}

void task3(void){
    int count;
    
    printf("Kuinka monta kokonaislukua haluat kasitella?: ");

    if(scanf("%d", &count) != 1){
        printf("Virheellinen syote.\n");
        return;
    }

    if(count <= 0){
        printf("Lukujen maaran tulee olla positiivinen.\n");
        return;
    }

    int *numbers = malloc(count * sizeof(int));

    if(numbers == NULL){
        printf("Muistin varaaminen epaonnistui.\n");
        return;
    }

    for(int i = 0; i < count; i++){
        printf("Anna luku %d: ", i + 1);

        if(scanf("%d", &numbers[i]) != 1){
            printf("Virheellinen syote.\n");
            free(numbers);
            return;
        }
    }

    int sum = 0;

    for(int i = 0; i < count; i++){
        sum += numbers[i];
    }

    double average = find_average(numbers, count);

    printf("Summa: %d\n", sum);
    printf("Keskiarvo: %.2f\n", average);


    free(numbers);
}

void task4(char *filename){
    FILE *file = fopen(filename, "r");

    if(file == NULL){
        printf("Tiedoston avaaminen epaonnistui.\n");
        return;
    }

    printf("Tiedosto avattiin onnistuneesti.\n");

    struct Student student;

    int count = 0;
    int grade_sum = 0;
    int best_grade;
    int best_id;

    while(fscanf(file, "%d %d", &student.id, &student.grade) == 2){
        printf("ID: %d, arvosana: %d\n", student.id, student.grade);
        if(count == 0){
            best_id = student.id;
            best_grade = student.grade;
        }
        else if(student.grade > best_grade){
            best_id = student.id;
            best_grade = student.grade;
        }

        count++;
        grade_sum += student.grade;
    }

    double average = 0;
    
    if(count > 0){
        average = (double)grade_sum / count;
        
        printf("Arvosanojen keskiarvo: %.2f\n", average);
        printf("Parhaan opiskelijan ID: %d ja arvosana: %d\n", best_id ,best_grade);
        
    }

    
    printf("Haettiin %d opiskelijan tiedot.\n", count);

    fclose(file);
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Anna tehtavan numero.\n");
        return 1;
    }

    int task = atoi(argv[1]);

    switch (task)
    {
    case 1:
        task1();
        break;
    case 2:
        task2();
        break;
    case 3:
        task3();
        break;
    case 4:
        if(argc < 3){
            printf("Anna tiedostonimi.\n");
            break;
        }
        task4(argv[2]);
        break;
    
    default:
        printf("Virheellinen tehtavanumero.\n");
        break;
    }

    return 0;
}
