// lineage.cpp
// purpose: implement recursive solutions to four data query problems
// author: Arman Kassam (akassa01)
// date: March 27th, 2024
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
struct Gene;

struct Mutation {
    int cost;
    Gene *target;
};

struct Gene {
    string sq; // Name of gene sequence
    bool seen; // Used in recursive functions to avoid loops
    Mutation mut;
};

const string EVOLVE = "e";
const string E_STEPS = "es";
const string ENE_EVOLVE = "ene";
const string PATH = "path";
const string QUIT = "q";

Gene *read_genes(string filename, int *num_genes);
void populate_nodes(Gene *genes, ifstream *inf, int num_nodes);
void populate_links(Gene *genes, ifstream *inf, int num_nodes);
int index_of(Gene *genes, string seq, int num_nodes);
void init(Gene *genes, int num_nodes);
void prompt_and_execute(Gene *all_genes, int num_genes);
bool can_evolve(Gene *src, Gene *tgt);
void handle_e(Gene *all_genes, int num_genes);
int evolution_steps(Gene *src, Gene *tgt);
void handle_es(Gene *all_genes, int num_genes);
int evolution_cost(Gene *src, Gene *tgt);
void handle_ene(Gene *all_genes, int num_genes);
string evolution_path(Gene *src, Gene *tgt);
void handle_path(Gene *all_genes, int num_genes);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cerr << "ERROR: No filename provided as an argument" << endl;
        exit(EXIT_FAILURE);
    }

    //After these two lines execute, all_genes is a pointer to the array of gene
    //information read in from a given data file, and num_genes contains the
    //number of genes stored in that array.
    int num_genes;
    Gene *all_genes = read_genes(argv[1], &num_genes);

    prompt_and_execute(all_genes, num_genes);
    
    delete[] all_genes;

    return 0;
}

void prompt_and_execute(Gene *all_genes, int num_genes) {
    string input;
    cout << "Enter a query: ";
    cin >> input;

    while (input != "q") {
        for (int i = 0; i < num_genes; i++) all_genes[i].seen = false;
        
        if (input == "e") handle_e(all_genes, num_genes);
        else if (input == "es") handle_es(all_genes, num_genes);
        else if (input == "ene") handle_ene(all_genes, num_genes);
        else if (input == "path") handle_path(all_genes, num_genes);
 
        // print error message if invalid input
        else cout << input << " not recognized." << endl << endl;
        cout << "Enter a query: ";
        cin >> input;
    }
}

// read_genes
// Input: A file name and a pointer to store the number of genes in the file.
// Description: Read in a file provided the user and use the data to populate an
//              array of Genes.
// Output: Returns a pointer to an array of Genes (populated with the contents
//         of a data file), and indirectly returns the number of array elements
//         through the num_genes pointer.
Gene *read_genes(string filename, int *num_genes)
{
    ifstream inf;
    inf.open(filename);
    if (inf.fail()) {
        cout << "ERROR OPENING FILE: Exiting Program" << endl;
        exit(EXIT_FAILURE);
    }

    inf >> *num_genes;

    // Create and populate the the names in the array of genes
    Gene *dna_p = new Gene[*num_genes];
    init(dna_p, *num_genes);
    populate_nodes(dna_p, &inf, *num_genes);

    // Reset the file to read in the mutation information
    inf.close();
    inf.open(filename);
    inf >> *num_genes; // NEED TO READ "past" NUMBER ON FIRST LINE

    populate_links(dna_p, &inf, *num_genes);

    inf.close();

    return dna_p;
}

// init
// Input: A pointer to an uninitialized array of genes and the size of
//        that array.
// Description: Initializes the array of genes with non-garbage information.
// Output: Initializes the array pointed to by 'dna'.
void init(Gene *dna, int num_nodes)
{
    for (int i = 0; i < num_nodes; i++) {
        dna[i].sq = "";
        dna[i].seen = false;
        dna[i].mut.cost = -1;
        dna[i].mut.target = nullptr;
    }
}

// populate_nodes
// Input: A pointer to an array of genes, a pointer to a file stream,
//        and the size of the array.
// Description: Read the file pointed to and set the sequence name of
//              each gene in the array.
// Output: Populates the sequence information for the gene array pointed to by
//         'dna'. Moves the file pointer further into the file.
void populate_nodes(Gene *dna, ifstream *inf, int num_nodes)
{
    string seq, mut_seq;
    int num_muts, num;
    for (int i = 0; i < num_nodes; i++) {
        *inf >> seq >> num_muts;
        dna[i].sq = seq;
        for (int j = 0; j < num_muts; j++) {
            *inf >> mut_seq >> num;
        }
    }
}

// populate_links
// Input: A pointer to an array of genes, a pointer to a file stream,
//        and the size of the array.
// Description: Read the file pointed to and populate the mutation
//              information between genes in the array. We assume the data file
//              only contains well-formed input.
// Output: Populates the mutation information for the gene array pointed to by
//         'dna'. Moves the file stream pointed to further into the file.
void populate_links(Gene *dna, ifstream *inf, int num_nodes)
{
    string seq, mut_seq;
    int num_muts;
    for (int i = 0; i < num_nodes; i++) {
        *inf >> seq >> num_muts;

        // Add the one mutation if it exists
        if (num_muts == 1) {
            int cost;
            *inf >> mut_seq >> cost;

            int index = index_of(dna, mut_seq, num_nodes);

            dna[i].mut.cost = cost;
            dna[i].mut.target = &dna[index];
        }
    }
}

// index_of
// Input: An array of genes, a gene sequence, and the size of the array.
// Description: Find the index of the given sequence in the array.
// Output: The index of the sequence in the array, or -1 if the sequence is not
//         in the array.
int index_of(Gene *dna, string seq, int num_nodes)
{
    for (int i = 0; i < num_nodes; i++) {
        if (dna[i].sq == seq) {
            return i;
        }
    }
    return -1;
}

/* handle_e
Input: A pointer to the struct array and an integer num_genes
Description: Match source and target and produce the correct output 
based on the result of the evolution possibility function
Output: none
*/
void handle_e(Gene *all_genes, int num_genes) {
    // create pointers to store the source and target genes
    Gene *src; 
    string input2;
    cin >> input2;
    int index = index_of(all_genes, input2, num_genes);
    src = &all_genes[index];
       
    Gene *tgt;
    string input3;
    cin >> input3;
    index = index_of(all_genes, input3, num_genes);
    tgt = &all_genes[index];

    if (can_evolve(src, tgt)) cout << src->sq 
    << " can evolve into " << tgt->sq << endl << endl;
    else cout << src->sq << " cannot evolve into " << tgt->sq << endl << endl;
}

/* can_evolve
Input: Pointers to the starting source and target gene elements
Description: use recursion to check if the evolution is possible
Output: a boolean variable 
*/
bool can_evolve(Gene *src, Gene *tgt) {
    
    /* base cases - if we've reached the target return true, otherwise if 
    there is nowhere to go or if we're entering a loop, return false */
    if (src->mut.target == nullptr) return false;
    else if (src->mut.target->sq == tgt->sq) return true;
    else if (src->seen) return false;
    
    // mark the current source as seen and continue to the next gene for answers
    else {
        src->seen = true;
        return can_evolve(src->mut.target, tgt);
    }
}

/* handle_es
Input: A pointer to the struct array and an integer num_genes
Description: Match source and target and produce the correct output 
based on the result of the evolution steps function
Output: none
*/
void handle_es(Gene *all_genes, int num_genes) {
    // make pointers to store the source and target genes
    Gene *src; 
    string input2;
    cin >> input2;
    int index = index_of(all_genes, input2, num_genes);
    src = &all_genes[index];
       
    Gene *tgt;
    string input3;
    cin >> input3;
    index = index_of(all_genes, input3, num_genes);
    tgt = &all_genes[index];

    // if the number of steps is greater than zero, ie. the path is valid, 
    // print the corresponding statement, otherwise print -1 in place of steps
    int steps = evolution_steps(src, tgt);
    if (steps > 0) cout << "It will take " <<  steps << 
    " evolutionary steps to get from " << src->sq << " to " << tgt->sq;
    else cout << "It will take -1 evolutionary steps to get from " << src->sq
    << " to " << tgt->sq;
    
    cout << endl << endl;
}

/* evolution_steps
Input: Pointers to the starting source and target gene elements
Description: figure out how many steps a given evolutionary path requires
Output: an integer containing the number of steps the evolution takes
*/
int evolution_steps(Gene *src, Gene *tgt) {
    
    /* base cases - if we've reached the target return 1, otherwise if 
    there is nowhere to go or if we're entering a loop, return a small 
    number to signal to the handle_es function that it is impossible */
    if (src->mut.target == nullptr) return -10000000; 
    else if (src->mut.target->sq == tgt->sq) return 1;
    else if (src->seen) return -10000000; 
    
    // mark the current source as seen and recursively add the number of steps
    else {
        src->seen = true;
        return 1 + evolution_steps(src->mut.target, tgt);
    }
}

/* handle_ene
Input: A pointer to the struct array and an integer num_genes
Description: Match source and target and produce the correct output 
based on the result of the evolution cost function
Output: none
*/
void handle_ene(Gene *all_genes, int num_genes) {
    
    // create pointers to store the source and target genes
    Gene *src; 
    string input2;
    cin >> input2;
    int index = index_of(all_genes, input2, num_genes);
    src = &all_genes[index];
       
    Gene *tgt;
    string input3;
    cin >> input3;
    index = index_of(all_genes, input3, num_genes);
    tgt = &all_genes[index];

    // store the cost the user gives
    int cost;
    cin >> cost;

    // if the evolution is possible with the given cost, print the 
    // corresponding statement 
    if (evolution_cost(src, tgt) <= cost) cout << src->sq << " can evolve into " 
    << tgt->sq << " with at most " << cost << " evolutionary cost\n" << endl;
    
    // if it's not possible with the given cost or at all, print the 
    // corresponding statement
    else cout << src->sq << " cannot evolve into " << tgt->sq << 
    " with at most " << cost << " evolutionary cost\n" << endl;

}

/* evolution_cost
Input: Pointers to the starting source and target gene elements
Description: add together the costs of all mutations on a given evolution path
Output: an integer containing the cost of the given evolution
*/
int evolution_cost(Gene *src, Gene *tgt) {
    // store the cost of the first mutation in a variable
    int n = src->mut.cost;

    /* base cases - if we've reached the target return the final cost that 
    needs to be added, otherwise if there is nowhere to go or if we're 
    entering a loop, return a large number to override the user-given cost */
    if (src->mut.target == nullptr) return 10000000; 
    else if (src->mut.target->sq == tgt->sq) return n;
    else if (src->seen) return 10000000; 
    
    // mark the current source as seen and recursively add the mutation costs
    else {
        src->seen = true;
        return n + evolution_cost(src->mut.target, tgt);
    }
}

/*  handle_path
Input: A pointer to the struct array and an integer num_genes
Description: Match source and target and produce the correct output 
based on the result of the path function
Output: none
*/
void handle_path(Gene *all_genes, int num_genes) {
    
    // create pointers to store the source and target genes
    Gene *src; 
    string input2;
    cin >> input2;
    int index = index_of(all_genes, input2, num_genes);
    src = &all_genes[index];
       
    Gene *tgt;
    string input3;
    cin >> input3;
    index = index_of(all_genes, input3, num_genes);
    tgt = &all_genes[index];

    /* if the path has the invalid marker, print the corresponding statement 
    otherwise print the full path */
    string evo_path = evolution_path(src, tgt);
    if (evo_path[evo_path.length() - 1] == 'X') {
        cout << "There is no path from " << src->sq << " to " << tgt->sq;
    }
    else cout << evo_path;

    cout << endl << endl;
}

/* evolution_path
Input: Pointers to the starting source and target gene elements
Description: use recursion to both figure out if a path is possible between 
source and target and create the path from beginning to end
Output: a string containing the path
*/
string evolution_path(Gene *src, Gene *tgt) {
    string whole_path = src->sq;
   
    /* base cases - if we've reached the target return the final element 
    of the string, otherwise if there is nowhere to go or if we're 
    entering a loop, return a marker to tell the algorithm it's invalid */
    if (src->sq == tgt->sq) return tgt->sq;
    else if (src->mut.target == nullptr) return "X"; 
    else if (src->seen) return "X"; 

    // mark the current source as seen and add recursively add on to the string
    else {
        src->seen = true;
        whole_path += " -> "; 
        return whole_path += evolution_path(src->mut.target, tgt);
    }
}