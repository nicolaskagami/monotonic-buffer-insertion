
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>

#define MAX_LINE 255
#define MAX_LABEL 32

class Point
{
    public:
        unsigned x;
        unsigned y;

        bool operator== (Point b);
};
typedef struct
{
    Point position;
    unsigned delay;//can be placed at the target vertex

    unsigned pindex;
    unsigned positive_targets;
    unsigned nindex;
    unsigned negative_targets;

}VERT;
typedef struct 
{
    unsigned target;

    unsigned path_delay;
    unsigned level;
}EDGE;

typedef struct
{
    unsigned vacant;
    unsigned inv_taken;
    unsigned signal_taken;
} LEVEL;

//PAAG data
typedef struct
{
    unsigned index;
    char name[MAX_LABEL];
    float delay;
} INPUT;

typedef struct
{
    unsigned index;
    char name[MAX_LABEL];
    float max_delay;
} OUTPUT;

typedef struct
{
    char name[MAX_LABEL];
    float period;
} CLOCK;

class MBI
{
    public:
        VERT * vertices;
        unsigned num_vertices;
        EDGE * edges;
        unsigned num_edges;

        unsigned max_cell_fanout;
        unsigned max_inv_fanout;

        MBI(unsigned v,unsigned e);
        MBI(char * paagFileName,char * sdcFileName);
        ~MBI();
        int allocate_memory(unsigned v, unsigned e);
        void preallocate(unsigned src,unsigned tgt,bool signal);
        void indexify();
        void add_edge(unsigned src,unsigned tgt,bool signal);
        void set_delay(unsigned vert,float delay);
	void set_position(unsigned vert,unsigned x,unsigned y);
        void print();

        //Parser
	//PAAG
        unsigned M,I,L,O,A,X,Y;
	INPUT * paag_inputs;
	OUTPUT * paag_outputs;
        void parse_paag(char * paagFileName);
	std::list<CLOCK> clocks;
        void parse_sdc(char * sdcFileName);
        //
        void option1(unsigned vert);
    private:
        //Constructing auxiliary variables
        unsigned current_edge;
        unsigned current_vert;

         


};
