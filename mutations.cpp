/*
* mutations.cpp
* purpose: serve as a database of information about a set of gene data
* author: Arman Kassam
* last modified: March 6th, 2024
*/

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

////////////////////////////////////////////////////////////////////////////
////////////////////////// STRUCT DEFINITIONS //////////////////////////////
////////////////////////////////////////////////////////////////////////////

struct Gene;

struct Mutation {
    Gene *mutation_name;
    int energy;
};

struct Gene {
    string source;
    int possibilities;
    Mutation mutations[5];
};

////////////////////////////////////////////////////////////////////////////
///////////////////////// FUNCTION DECLARATIONS ////////////////////////////
////////////////////////////////////////////////////////////////////////////

void is_valid(string filename);
Gene *read_data(string filename, int *num_genes);
void init_data(Gene *genes, int num_genes);
void read_genes(string filename, Gene *genes, int num_genes);
void read_mutations(string filename, Gene *genes, int num_genes);
Gene *find_gene(Gene *genes, int num_genes, string name);
void prompt_and_execute(Gene *genes, int num_genes);
void print_data(Gene *genes, int num_genes);
string is_possible(Gene *starting);
void energy_cost(Gene *starting);

////////////////////////////////////////////////////////////////////////////
////////////////////////////////// MAIN ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    // make sure there are enough arguments
    if (argc < 2) {
        cerr << "ERROR: A filename must be specified as the first argument.\n";
        exit(EXIT_FAILURE);
    }

    int num_genes;
    is_valid(argv[1]);
    Gene *genes = read_data(argv[1], &num_genes);

    prompt_and_execute(genes, num_genes);

    // delete and return
    delete[] genes;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// FUNCTIONS ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/* is_valid
Inputs: the filename and the empty integer of starting genes
Purpose: fight back against the martians (elements 
interspersed throughout the code)!
Outputs: an error message and an exit, otherwise nothing
*/
void is_valid(string filename) {
    // check if it opens
    ifstream infile(filename);
    if (infile.fail()) {
        cerr << "ERROR: Error opening file, please check file name: "
             << filename << endl;
       exit(EXIT_FAILURE);
    }
    int lines, i = 1;
    infile >> lines;
    char next;
   
    infile.get(next);
    while (!infile.eof()) {
        infile.get(next);
        if (next == '\n') i++;
    }
    if (i != lines) {
        cerr << "Invalid file format. Exiting program." << endl;
        exit(3);
    }

    infile.close();
}
/* read_data 
Inputs: the filename and a pointer to the number of starting genes
Purpose: read the file data into the program
Outputs: a struct array full of the data
*/
Gene *read_data(string filename, int *num_genes) {
    ifstream infile(filename);

    // infile the number of starting genes, allocate heap space, and initialize
    infile >> *num_genes;
    infile.close();
    Gene *genes = new Gene[*num_genes];
    init_data(genes, *num_genes);

    // read the rest of the data in
    read_genes(filename, genes, *num_genes);
    read_mutations(filename, genes, *num_genes);
    
    return genes;
}

/* init_data
Inputs: a pointer to the struct array and the number of starting genes
Purpose: initialize all values of the struct array
Outputs: nothing
*/
void init_data(Gene *genes, int num_genes) {
    // initialize all parts of the struct array
    for (int i = 0; i < num_genes; i++) {
        genes[i].source = " ";
        genes[i].possibilities = 0;
        for (int j = 0; j < 5; j++) {
            genes[i].mutations[j].mutation_name = nullptr;
            genes[i].mutations[j].energy = 0;
        }
    }
}

/* read_genes
Inputs: the filename, a pointer to the gene 
array, and the number of starting genes
Purpose: fill in the two main parts of the gene struct array
Outputs: nothing 
*/
void read_genes(string filename, Gene *genes, int num_genes) {
    /* reopen the file and skip over the beginning 
    (no need to recheck if it opens because we already have) */
    ifstream infile(filename);
    infile >> num_genes;
    
    // create two variables to skip over the other data
    string skip_string;
    int skip_int;

    /* read in the source gene and the number of possibilities, 
    and skip the other parts of the line accordingly */
    for (int i = 0; i < num_genes; i++) {
        infile >> genes[i].source;
        // check if the genes use the proper characters
        for (int j = 0; j < 4; j++) {
            if (genes[i].source[j] != 'A' && genes[i].source[j] != 'G' 
            && genes[i].source[j] != 'C' && genes[i].source[j] != 'T') {
                cerr << "Invalid file format. Exiting program." << endl;
                exit(3);
            }
        }
        
        // check if the genes are the proper length
        int length = (genes[i].source).length();
        if (length > 4) {
            cerr << "Invalid file format. Exiting program." << endl;
            exit(3);
        }

        infile >> genes[i].possibilities;
        // make sure the possibilities number is above zero and below 5
        if (genes[i].possibilities < 0 || genes[i].possibilities > 5) {
            cerr << "Invalid file format. Exiting program." << endl;
            exit(3);
        }
        for (int j = 0; j <  genes[i].possibilities; j++) {
            infile >> skip_string;
            infile >> skip_int;
        }
    }
    // make sure no source is repeated
    for (int i = 0; i < num_genes; i++) {
        int repeat = 0; 
        for (int j = 0; j < num_genes; j++) {
            if (genes[i].source == genes[j].source) repeat++;
            if (repeat > 1) {
                cerr << "Invalid file format. Exiting program." << endl;
                exit(3);
            }
        }
    }
    infile.close();
}

/* read_mutations
Inputs: the filename, a pointer to the gene 
array, and the number of starting genes
Purpose: fill in the sub struct array of mutations using the file data
Outputs: nothing
*/
void read_mutations(string filename, Gene *genes, int num_genes) {
    // reopen the file and skip over the beginning
    ifstream infile(filename);
    infile >> num_genes;
    
    /* create variables to skip the first part of each 
    line, as well as store the name of the mutations */
    string skip_string, name;
    int skip_int;
    
    // read in the mutations and cost
    for (int i = 0; i < num_genes; i++) {
        infile >> skip_string;
        infile >> skip_int;
        for (int j = 0; j < skip_int; j++) {
            infile >> name;
            infile >> genes[i].mutations[j].energy;
            // make sure no energy value is below 0
            if (genes[i].mutations[j].energy < 0) {
                cerr << "Invalid file format. Exiting program." << endl;
                exit(3);
            }
            
            // find a pointer that correlates to the given name
            Gene *ptr = find_gene(genes, num_genes, name);
            // if it doesn't match any source gene, exit
            if (ptr == nullptr) {
                cerr << "Invalid file format. Exiting program." << endl;
                exit(3);
            }
            
            // put the pointer in the sub struct array in its proper place
            genes[i].mutations[j].mutation_name = ptr;
        }
    }

    infile.close();
}

/* find_gene
Inputs: a pointer to the struct array, the number of starting
genes, and a string of the name taken from the file 
Purpose: find the gene the user is trying to access
Outputs: a pointer to the correct element of the struct array 
*/
Gene *find_gene(Gene *genes, int num_genes, string name) {
    // find the gene array that has a source that matches the given name
    for (int i = 0; i < num_genes; i++) {
        if (name == genes[i].source) return &genes[i];
    }
    return nullptr;
}

/* prompt_and_execute
Inputs: a pointer to the struct array and the number of starting genes
Purpose: prompt the user for input and direct the program to the proper function
Outputs: nothing
*/
void prompt_and_execute(Gene *genes, int num_genes) {
    // take in the first argument on the command line
    string input;
    cout << "Enter a query: ";
    cin >> input;

    while (input != "q") {
        if (input == "p") print_data(genes, num_genes);
        else if (input == "m" || input == "me") {
            Gene *starting;
            string input2;
            cin >> input2;
            // turn the second input into a pointer to pass into the functions 
            for (int i = 0; i < num_genes; i++) {
                if (genes[i].source == input2) {
                    starting = &genes[i];
                    break;
                }
            }
            if (input == "m") is_possible(starting);
            if (input == "me") energy_cost(starting);
            cout << endl;
        }
        // print error message if invalid input
        else cout << input << " not recognized." << endl;
        cout << endl;
        cout << "Enter a query: ";
        cin >> input;
    }
}

/* print_data
Inputs: a pointer to the struct array and the number of starting genes
Purpose: print the information in the array
Outputs: nothing
*/
void print_data(Gene *genes, int num_genes) {
    // print the data nicely depending on how many possibilities each gene has
    for (int i = 0; i < num_genes; i++) {
        cout << "== " << genes[i].source << " ==\n";
        cout << "Mutations:\n";
        if (genes[i].possibilities < 1)  {
            cout << "None\n";
            continue;
        }
        for (int j = 0; j < genes[i].possibilities; j++) {
            cout << genes[i].mutations[j].mutation_name->source;
            cout << " - Cost: " << genes[i].mutations[j].energy << endl;
        }
    }
}

/* is_possible
Inputs: a pointer to the equivalent of the second input on the command line
Purpose: check if the mutation a starting gene and another is possible
Outputs: a string containing the third input (for the purposes of energy_cost)
*/
string is_possible(Gene *starting) {
    // cin the third input, or the mutated gene
    string mutated;
    cin >> mutated;

    // check with all possible mutations, and if correct, turn possible to true
    for (int i = 0; i < starting->possibilities; i++) {
        if (starting->mutations[i].mutation_name->source == mutated) {
            cout << starting->source << " can mutate into " << mutated;
            return mutated;
        }
    }

    cout << starting->source << " cannot mutate into " << mutated;
    return "no";
}

/* energy_cost
Inputs: a pointer to the equivalent of the second input on the command line
Purpose: check if the cost given is a valid amount for the mutation
Outputs: none
*/
void energy_cost(Gene *starting) {

    // run is_possible to get the first half of the sentence
    string mutated = is_possible(starting);
    
    // find the cost amount that matches the mutated gene
    int cost = 0;
    for (int i = 0; i < starting->possibilities; i++) {
        if (starting->mutations[i].mutation_name->source == mutated) {
            cost = starting->mutations[i].energy;
        }
    }
    
    // cin the fourth input and print a message based on if it's 
    // enough and if is_possible returned a positive input
    int resources;
    cin >> resources;
    if (cost <= resources && mutated != "no") {
        cout << " with evolutionary cost " << resources;
    }
    if (cost > resources && mutated != "no") {
        cout << " but not with evolutionary cost " << resources;
    }
}