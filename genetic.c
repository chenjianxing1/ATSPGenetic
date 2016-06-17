/*
 * Guilherme Alles e Rafael Valer
 *
 * An instance of the ATSP is represented by a NxN matrix
 * containing the costs from node i to node j.
 * Each possible solution is an array with N elements,
 * representing the order in which the nodes are visited.
 * Each node must appear exactly once in a valid solution.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Solutions population
#define POPULATION_SIZE 10

const int instance_size = 5;
const int instance[5][5] = {
    {999999, 100, 3, 3, 100},
    {100, 999999, 100, 3, 3},
    {3, 100, 999999, 100, 3},
    {3, 3, 100, 999999, 100},
    {100, 3, 3, 100, 999999}
};

int population[POPULATION_SIZE][5] = {
    {0, 1, 2, 3, 4},
    {1, 2, 3, 4, 0},
    {2, 3, 4, 0, 1},
    {3, 4, 0, 1, 2},
    {4, 0, 1, 2, 3},
    {3, 2, 1, 0, 4},
    {2, 1, 0, 3, 4},
    {1, 0, 4, 3, 2},
    {0, 4, 3, 1, 2},
    {0, 1, 2, 3, 4}
};

int *population_costs;
int *population_fitness;
double *cumulative_fitness;

void initializeRandomGenerator();
unsigned int calculateCost(unsigned int solution_index);
int validSolution (unsigned int solution_index);
void stepGeneration();
int notChosen(unsigned int node, int *solution);
unsigned int getFirstNotChosenNode (int *solution);
int* crossover (unsigned int parent1_index, unsigned int parent2_index);
void printSolution(int *solution);
void calculateFitness();
void printSolutionInPopulation(int solution_index);


int main(int argc, char **argv) {
    initializeRandomGenerator();

    population_costs = (int*) malloc (sizeof(int) * POPULATION_SIZE);
    population_fitness = (int*) malloc (sizeof(int) * POPULATION_SIZE);
    cumulative_fitness = (double*) malloc (sizeof(double) * POPULATION_SIZE);

    int i;
    for (i=0; i<30; i++) {
        stepGeneration();
        calculateFitness();
        int lowest_cost = population_costs[0];
        int lowest_cost_index = 0;
        int j;
        for (j=0; j<POPULATION_SIZE; j++) {
            if (population_costs[j] < lowest_cost) {
                lowest_cost = population_costs[j];
                lowest_cost_index = j;
            }
        }

        printf("\n@@@ Generation #%d:\n", i);
        printSolutionInPopulation(lowest_cost_index);
    }


    return 0;
}

void initializeRandomGenerator() {
    srand(time(NULL));
}

unsigned int calculateCost(unsigned int solution_index) {
    int i=0, j=0, total_cost=0;

    if (validSolution(solution_index)) {
        for (i=0; i<instance_size; i++) {
            j = (i+1) % instance_size;
            // Add the cost of going from node i to node j0
            total_cost += instance[population[solution_index][i]][population[solution_index][j]];
        }
    }
    else {
        return 999999;
    }
    return total_cost;
}

int validSolution (unsigned int solution_index) {
    int i=0, j=0, found=0;
    for (i=0; i<instance_size; i++) {
        for(j=0; j<instance_size; j++) {
            if (population[solution_index][j] == i) {
                found++;
            }
        }
        if (found != 1)
            return 0;
        else
            found = 0;
    }
    return 1;
}

int notChosen(unsigned int node, int *solution) {
    int i;
    for(i=0; i<instance_size; i++) {
        if (solution[i] == node)
            return 0;
        else if (solution[i] == -1)
            return 1;
    }
    return 1;
}

// Given a solution, return the first node that does not yet
// appear on the solution. Returns -1 if not found.
unsigned int getFirstNotChosenNode (int *solution) {
    int i;
    for (i=0; i<instance_size; i++) {
        if (notChosen(i, solution)) {
            return i;
        }
    }

    return -1;
}

// Returns a new child in memory. NOTE: The child which will
// be substituted MUST be freed!!
int* crossover (unsigned int parent1_index, unsigned int parent2_index) {
    int* child = malloc(sizeof(int) * instance_size);
    int i=0;
    for (i=0; i<instance_size; i++) {
        child[i]=-1;
    }

    // randomly selects the first city from one of
    // the parents
    int parent = rand()%2;
    child[0] = (parent==0) ? population[parent1_index][0] : population[parent2_index][0];

    for (i=1; i<instance_size; i++) {
        // Calculate the distances of the two cities leaving
        // the previous city
        int parent1_nextcity = population[parent1_index][i];
        int parent1_nextcity_cost = instance[child[i-1]][parent1_nextcity];

        int parent2_nextcity = population[parent2_index][i];
        int parent2_nextcity_cost = instance[child[i-1]][parent2_nextcity];

        if ((parent1_nextcity_cost<=parent2_nextcity_cost) && notChosen(parent1_nextcity, child)) {
            child[i] = parent1_nextcity;
        }
        else if ((parent2_nextcity_cost<parent1_nextcity_cost) && notChosen(parent2_nextcity, child)) {
            child[i] = parent2_nextcity;
        }
        else {
            int first = getFirstNotChosenNode(child);
            if (first == -1) {
                fprintf(stderr, "Isso não deve acontecer.\n");
                exit(EXIT_FAILURE);
            }
            child[i] = first;
        }
    }

    return child;
}

void calculateFitness() {
    int i;

    unsigned int costs_sum = 0;
    for (i=0; i<POPULATION_SIZE; i++) {
        population_costs[i] = calculateCost(i);
        costs_sum += population_costs[i];
    }

    unsigned int fitness_sum=0;
    for (i=0; i<POPULATION_SIZE; i++) {
        population_fitness[i] = costs_sum - population_costs[i];
        fitness_sum += population_fitness[i];
    }

    cumulative_fitness[0] = population_fitness[0]/fitness_sum;
    for (i=1; i<POPULATION_SIZE; i++) {
        cumulative_fitness[i] = cumulative_fitness[i-1] + (population_fitness[i]/fitness_sum);
    }

}

// Selects a solution from the population considering the
// cumulative fitness as a likelihood measure. The restriction
// parameter is used to avoid selecting the same solution twice
unsigned int selectSolutionFromFitness(int restriction) {
    double random = (rand()%100)/100;
    if ((random >= 0) && (random < cumulative_fitness[0]))
        return 0;

    int i;
    for (i=1; i<POPULATION_SIZE; i++) {
        if ((random >= cumulative_fitness[i-1]) && (random < cumulative_fitness[i])) {
            if (i != restriction)
                return i;
            else
                return selectSolutionFromFitness(restriction);
        }
    }

    return POPULATION_SIZE-1;
}

// Returns the index of the least fit solution in the population
unsigned int selectLeastFit() {
    int lowest_fitness = population_fitness[0];
    unsigned int least_fit_index = 0;
    unsigned int i;
    for (i=1; i<POPULATION_SIZE; i++) {
        if (population_fitness[i] < lowest_fitness) {
            lowest_fitness = population_fitness[i];
            least_fit_index = i;
        }
    }
    return least_fit_index;
}

void printSolution(int *solution) {
    printf("Solution: ");
    int i, total_cost = 0;
    for(i=0; i<instance_size-1; i++) {
        if (i>0)
            total_cost += instance[solution[i-1]][solution[i]];
        printf("%d ->", solution[i]);
    }
    total_cost += instance[solution[i-1]][solution[i]];
    printf("%d.\nTotal cost: %d.\n", solution[i], total_cost);
}

void printSolutionInPopulation(int solution_index) {
    printf("Solution: ");
    int i, total_cost = 0;
    for(i=0; i<instance_size-1; i++) {
        if (i>0)
            total_cost += instance[population[solution_index][i-1]][population[solution_index][i]];
        printf("%d ->", population[solution_index][i]);
    }
    total_cost += instance[population[solution_index][i-1]][population[solution_index][i]];
    printf("%d.\nTotal cost: %d.\n", population[solution_index][i], total_cost);
}

void mutate(int *solution) {
    int swap_node_1 = rand() % instance_size;
    int swap_node_2 = rand() % instance_size;

    int tmp = solution[swap_node_1];
    solution[swap_node_1] = solution[swap_node_2];
    solution[swap_node_2] = solution[swap_node_1];
}

// Advance to the next generation
void stepGeneration() {
    calculateFitness();
    int parent1_index = selectSolutionFromFitness(-1);
    int parent2_index = selectSolutionFromFitness(parent1_index);

    int *child = crossover(parent1_index, parent2_index);

    int mutation_prob = rand() % 100;
    if (mutation_prob <= 1) {
        mutate(child);
    }

    int solution_to_die = selectLeastFit();
    int i;
    // Substitute a less fit solution with the new child.
    for (i=0; i<instance_size; i++) {
        population[solution_to_die][i] = child[i];
    }

    free(child);
}
