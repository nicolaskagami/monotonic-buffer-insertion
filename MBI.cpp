
#include "MBI.h"

MBI::MBI(int argc,char ** argv)
{
	unsigned positional_input_source = 0;
	char * positional_input_file_name = NULL; //PAAG or DEF
	char * timing_input_file_name 	  = NULL; //SDC Input Output Timing
	char * cell_input_file_name 	  = NULL; //LIB Cell Library
	for(unsigned i=1;i<argc;i++)
	{
		if(strcmp("--paag",argv[i])==0)
		{
			positional_input_file_name = argv[++i];
			positional_input_source = 1;
		}else
		if(strcmp("--def",argv[i])==0)
		{
			positional_input_file_name = argv[++i];
			positional_input_source = 2;
		}else
		if(strcmp("--sdc",argv[i])==0)
		{
			timing_input_file_name = argv[++i];
		}else
		if(strcmp("--lib",argv[i])==0)
		{
			cell_input_file_name = argv[++i];
		}
	}
	if((positional_input_file_name)&&(timing_input_file_name)&&(cell_input_file_name))
	{
		printf("Selected positional input file:   \t%s\n",positional_input_file_name);
		printf("Selected timing input file:       \t%s\n",timing_input_file_name);
		printf("Selected cell library input file: \t%s\n",cell_input_file_name);
		switch(positional_input_source)
		{
			case 1: parse_paag(positional_input_file_name);break;
			case 2: parse_def(positional_input_file_name);exit(1);break;//def parser
			default:exit(1);
		}
		parse_sdc(timing_input_file_name);
		set_clock();
		lib = new Liberty(cell_input_file_name);
	}
	else
	{
		if(!positional_input_file_name)
		{
			printf("MBI Error: No positional input file\n");
			printf("\tDefine positional file as --def defFileName.def or --paag paagFileName.paag\n");
		}
		if(!timing_input_file_name)
		{
			printf("MBI Error: No timing input file\n");
			printf("\tDefine timing file as --sdc sdcFileName.sdc\n");
		}
		if(!cell_input_file_name)
		{
			printf("MBI Error: No Cell library input file\n");
			printf("\tDefine cell library file as --lib libFileName.lib\n");
		}
		exit(1);
	}
}

MBI::~MBI()
{
	for(unsigned i=0;i<num_vertices;i++)
    {
        if(vertices[i].inverter_tree)
            delete(vertices[i].inverter_tree);
    }
	
    free(vertices);
    free(edges);
    free(inputs);
    free(outputs);
    clean_sdc();
    clean_paag();
    if(lib)
        delete(lib);
}

void MBI::print()
{
    unsigned i;
    printf("Clocks:\n");
    for (std::list<CLOCK>::iterator it=clocks.begin(); it!=clocks.end(); ++it)
       printf("%s: Period: %f\n",it->name,it->period); 
    printf("----------------------------------------------------------------------\n");
    printf("Inputs\n");
    for(i=0;i<num_inputs;i++)
        printf("IN %d %s: %d Delay: %f\n",i,inputs[i].name,inputs[i].index,inputs[i].delay);
    printf("Outputs\n");
    for(i=0;i<num_outputs;i++)
            printf("OUT %d %s: %d Max Delay: %f\n",i,outputs[i].name,outputs[i].index,outputs[i].max_delay);
    printf("----------------------------------------------------------------------\n");
    printf("Vertices: %d\n",num_vertices);
    printf("Edges: %d\n",num_edges);
    for(i = 0;i<num_vertices;i++)
    {
        if((vertices[i].positive_targets+vertices[i].negative_targets)>0)
        {
            printf("Vert %d (%.2f,%.2f)\n",i,vertices[i].position.x,vertices[i].position.y);
            printf("Sources: ");
            for(unsigned srcs = 0; srcs < vertices[i].num_srcs;srcs++)
                printf("%u ",vertices[i].srcs[srcs]);
            printf("\nEstimated Delay: %f %f\n",vertices[i].pre_delay,vertices[i].post_delay);
            printf("Positive Consumers: ");
            for(unsigned b = vertices[i].pindex,j=0;j<vertices[i].positive_targets;j++)
            {
                if(j<vertices[i].num_positive_critical)
                    printf("[%d] ",edges[b+j].target);
                else
                    printf("%d ",edges[b+j].target);
            }
            printf("\nNegative Consumers: ");
            for(unsigned b = vertices[i].nindex,j=0;j<vertices[i].negative_targets;j++)
            {
                if(j<vertices[i].num_negative_critical)
                    printf("[%d] ",edges[b+j].target);
                else
                    printf("%d ",edges[b+j].target);
            }
            printf("\n");
			if(vertices[i].inverter_tree)
			vertices[i].inverter_tree->print();
        }
    }
}
//Parsers
//Zeroth vertex is true, negated is false
void MBI::parse_paag(char * paagFileName)
{
	paag = new Paag(paagFileName);

	vertices = paag->topology->vertices;
	num_vertices = paag->topology->num_vertices;
	
	edges = paag->topology->edges;
	num_edges = paag->topology->num_edges;
	
	inputs = paag->topology->inputs;
	num_inputs = paag->topology->num_inputs;
	outputs = paag->topology->outputs;
	num_outputs = paag->topology->num_outputs;
}
void MBI::clean_paag()
{
    if(paag)
		delete(paag);
}
void MBI::parse_def(char * defFileName)
{
	
}
void MBI::clean_def()
{
	
}
void MBI::parse_sdc(char * sdcFileName)
{
    FILE * sdcFile;
    sdcFile = fopen(sdcFileName,"r");
    if(sdcFile)
    {
        char line[MAX_LINE];
        char * aux;
        //Header
        while(fgets(line,MAX_LINE,sdcFile))
        {
            aux = strtok(line," \t");
            if(strcmp(aux,"create_clock") == 0)
            {
                CLOCK clk;
                for(aux=strtok(NULL," \n");aux!=NULL;aux=strtok(NULL," \n"))
                {
                    if(strcmp(aux,"-period")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            clk.period = strtof(aux,NULL);
                            //printf("Period: %f\n",clk.period);
                        }
                    }
                    if(strcmp(aux,"-name")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            strcpy(clk.name,aux);
                            //printf("NAME: %s\n",clk.name);
                        }
                    }
                }
                clocks.push_back(clk); 
            }
            else
            if(strcmp(aux,"set_input_delay") == 0)
            {
                float delay;
                bool delay_parsed = false;
                char input_name[MAX_LINE];
                for(aux=strtok(NULL," \n");aux!=NULL;aux=strtok(NULL," \n"))
                {
                    if(strcmp(aux,"-clock")==0)
                    {
                        aux=strtok(NULL," \n");
                        if(aux)
                        {
                            //printf("For clock: %s\n",aux);
                        }
                    } else
                    if(delay_parsed==false)
                    {
                        delay = strtof(aux,&aux);
                        delay_parsed = true;
                    } else {
                        if(aux)
                        {
                            unsigned i;
                            strcpy(input_name,aux);
                            for(i=0;i<num_inputs;i++)
                            {
                                if(strcmp(input_name,inputs[i].name)==0)
                                {
                                    inputs[i].delay = delay;
                                    //set_delay(inputs[i].index,delay);        
                                }
                            }
                        }
                    }                        
                }
            }
            else
            if(strcmp(aux,"set_max_delay") == 0)
            {
                float delay;
                bool delay_parsed = false;
                char output_name[MAX_LINE];
                for(aux=strtok(NULL," \t\n");aux!=NULL;aux=strtok(NULL," \t\n"))
                {
                    if(strcmp(aux,"-to")==0)
                    {
                        aux=strtok(NULL," \t\n");
                        if(aux)
                        {
                            unsigned i;
                            strcpy(output_name,aux);
                            for(i=0;i<num_outputs;i++)
                            {
                                if(strcmp(output_name,outputs[i].name)==0)
                                {
                                    outputs[i].max_delay = delay;
                                    //set_delay(outputs[i].index,delay);        
                                }
                            }
                        }
                    } else
                    if(delay_parsed==false)
                    {
                        delay = strtof(aux,&aux);
                        delay_parsed = true;
                    }
                }
            }
        }
        fclose(sdcFile);
    }

}
void MBI::clean_sdc()
{
    
}
void MBI::set_clock()
{
    if(clocks.size()!=0)
    {
        current_clock = clocks.front();
    }
    else
    {
        printf("SDC Error: No Clocks Set\n");
    }
}
//
void MBI::estimate_delay()
{
    unsigned i,vert;
    std::queue<unsigned> q;
    float delay;
    //Input Propagation
    for(i=0;i<num_inputs;i++)
    {
        vert = inputs[i].index;
        if(vertices[vert].pre_delay<inputs[i].delay)
            vertices[vert].pre_delay = inputs[i].delay;
        q.push(vert);
    }
    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        unsigned ind,base,tgt;
        for(base = vertices[vert].pindex,ind=0;ind<(vertices[vert].positive_targets+vertices[vert].negative_targets);ind++)
        {
            tgt = edges[base+ind].target;
            delay = vertices[vert].pre_delay + nodal_delay;
            if(vertices[tgt].pre_delay<delay)
                vertices[tgt].pre_delay = delay;
            q.push(tgt);
        }
    }
    //Output Propagation
    for(i=0;i<num_outputs;i++)
    {
        vert = outputs[i].index;
        if(outputs[i].max_delay>current_clock.period)
        {
            printf("SDC Warning: Main Clock Period lower than an output's maximum delay\n");
            printf("\tClock: %s \tPeriod: %f -> %f from output %s\n",current_clock.name, current_clock.period,outputs[i].max_delay,outputs[i].name);
            current_clock.period = outputs[i].max_delay;
        }
        delay = current_clock.period - outputs[i].max_delay;
        if(vertices[vert].post_delay<delay)
            vertices[vert].post_delay = delay;
        q.push(vert);
    }
    while(!q.empty())
    {
        vert = q.front();
        q.pop();
        unsigned i,src;
        for(i=0;i<vertices[vert].num_srcs;i++)
        {
            src = vertices[vert].srcs[i];
            delay = vertices[vert].post_delay + nodal_delay;
            if(vertices[src].post_delay<delay)
                vertices[src].post_delay = delay;
            q.push(src);
        }
    }
}
void MBI::insert_buffers()
{
    //Ways to run over all vertices:
    //1: Iterate over the array
    //2: Propagate from inputs 
    //3: Propagate from outputs 
    //
    //sort the edges
    for(unsigned i=0;i<num_vertices;i++)
    {

		if(min_height(vertices[i].positive_targets,vertices[i].negative_targets)>0)
		{
			//Sort the targets
			//sort_vert(i);
			sort_vert(vertices[i]);
			//Determine the critical ones (a number of how many of the first positive and negative are critical)
			select_criticals(i);
			//Allocate
			vertices[i].inverter_tree = new InverterTree(vertices[i].positive_targets,vertices[i].negative_targets,max_cell_fanout,max_inv_fanout,inv_delay,vertices[i].position);
			//
			add_criticals(i);
			vertices[i].inverter_tree->expand();
			
			//
			add_non_criticals(i);
			vertices[i].inverter_tree->connect_positioned_targets();
			vertices[i].inverter_tree->print_inverters();
		}
	}
}
void MBI::sort_vert(VERT vert)
{
    EDGE * paux;
    EDGE * naux;

    naux = (EDGE*) malloc(vert.negative_targets*sizeof(EDGE));
    paux = (EDGE*) malloc(vert.positive_targets*sizeof(EDGE));
    
    //Ready to parallelize
    mSort(&(edges[vert.pindex]),paux,0,vert.positive_targets-1);
    mSort(&(edges[vert.nindex]),naux,0,vert.negative_targets-1);
    
    free(naux);
    free(paux);
}
void MBI::sort_vert(unsigned vert)
{
    EDGE * paux;
    EDGE * naux;

    naux = (EDGE*) malloc(vertices[vert].negative_targets*sizeof(EDGE));
    paux = (EDGE*) malloc(vertices[vert].positive_targets*sizeof(EDGE));
    //Ready to parallelize
    mSort(&(edges[vertices[vert].pindex]),paux,0,vertices[vert].positive_targets-1);
    mSort(&(edges[vertices[vert].nindex]),naux,0,vertices[vert].negative_targets-1);
    
    free(naux);
    free(paux);
}
void MBI::select_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;
    float highest_delay = 0;
    
    if(vertices[vert].positive_targets!=0)
        highest_delay = vertices[edges[pbase].target].post_delay;
    if((vertices[vert].negative_targets!=0)&&(vertices[edges[nbase].target].post_delay>highest_delay))
        highest_delay = vertices[edges[nbase].target].post_delay;
        
    if(highest_delay==0)
        return;

    //Paralellizable
    vertices[vert].num_positive_critical = vertices[vert].positive_targets;//In case all are critical...
    for( unsigned i=0;i<vertices[vert].positive_targets;i++)
    {
        if(vertices[edges[pbase+i].target].post_delay<highest_delay*CRITICAL_THRESHOLD)
        {
            vertices[vert].num_positive_critical = i;
            break;
        }
    }
    
    vertices[vert].num_negative_critical = vertices[vert].negative_targets;//In case all are critical...
    for( unsigned i=0;i<vertices[vert].negative_targets;i++)
    {
        if(vertices[edges[nbase+i].target].post_delay<highest_delay*CRITICAL_THRESHOLD)
        {
            vertices[vert].num_negative_critical = i;
            break;
        }
    }
}
void MBI::add_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
	
    for( unsigned i=0;i<vertices[vert].num_positive_critical;i++)
    {
		vertices[vert].inverter_tree->add_critical_target(edges[pbase+i].target,false,vertices[edges[pbase+i].target].post_delay);
    }
    
    for( unsigned i=0;i<vertices[vert].num_negative_critical;i++)
    {
		vertices[vert].inverter_tree->add_critical_target(edges[nbase+i].target,true,vertices[edges[nbase+i].target].post_delay);
    }
}
void MBI::add_non_criticals(unsigned vert)
{
    unsigned pbase = vertices[vert].pindex;
    unsigned nbase = vertices[vert].nindex;

    //Paralellizable
	
    for( unsigned i=vertices[vert].num_positive_critical;i<vertices[vert].positive_targets;i++)
    {
		vertices[vert].inverter_tree->add_positive_target(edges[pbase+i].target,true,vertices[edges[pbase+i].target].post_delay,vertices[edges[pbase+i].target].position);
    }
    
    for( unsigned i=vertices[vert].num_negative_critical;i<vertices[vert].negative_targets;i++)
    {
		vertices[vert].inverter_tree->add_negative_target(edges[nbase+i].target,true,vertices[edges[nbase+i].target].post_delay,vertices[edges[nbase+i].target].position);
    }
}
void MBI::set_nodal_delay(char * cellName,char * invName)
{
    for (std::list<CELL>::iterator it=lib->cells.begin(); it!=lib->cells.end(); ++it)
    {
        if(strcmp(cellName,it->name)==0)
        {
            nodal_delay = 0.005;
        }else
        if(strcmp(invName,it->name)==0)
        {
            inv_delay = 0.001;
        }
    }
}

void MBI::merge(EDGE * a,EDGE *aux,int left,int right,int rightEnd)
{
    int i,num,temp,leftEnd;
    leftEnd = right - 1;
    temp = left;
    num = rightEnd - left + 1;
    while((left<=leftEnd)&&(right<=rightEnd))
    {
        if(vertices[a[left].target].post_delay > vertices[a[right].target].post_delay)
           aux[temp++]=a[left++];
        else
            aux[temp++]=a[right++];
    }
    while(left <= leftEnd)
        aux[temp++]=a[left++];

    while(right <= rightEnd)
        aux[temp++]=a[right++];

    for(i=1;i<=num;i++,rightEnd--)
        a[rightEnd] = aux[rightEnd];
}
void MBI::mSort(EDGE * a,EDGE *aux,int left,int right)
{
    int center;
    if(left<right)
    {
        center=(left+right)/2;
        mSort(a,aux,left,center);
        mSort(a,aux,center+1,right);
        merge(a,aux,left,center+1,right);
    }
}
unsigned MBI::min_height(unsigned posConsumers,unsigned negConsumers)
{
    unsigned posAvailable = max_cell_fanout;
    unsigned negAvailable = 0;
    unsigned minHeight = 0;
    unsigned leavesAvailable, leaves;
    unsigned height1_branches;
    
    if((negConsumers == 0)&&(posConsumers<=max_inv_fanout)) 
    {
        return 0; 
    }
    do
    {
        minHeight++;
        if(minHeight%2)
        {
            //New layer is odd (negative)
            negAvailable=posAvailable*max_inv_fanout;
            leaves = negConsumers;
            height1_branches = posConsumers;
            leavesAvailable = negAvailable;
        }
        else
        {
            //New layer is even (positive)
            posAvailable=negAvailable*max_inv_fanout;
            leaves = posConsumers;
            height1_branches = negConsumers;
            leavesAvailable = posAvailable;
        }
        //printf("Height:%d Available P %d, N %d \n",minHeight,posAvailable,negAvailable);
    }
    while((leaves > leavesAvailable)||(height1_branches > ((leavesAvailable-leaves)/max_inv_fanout)));
	
    return minHeight;
}


int main(int argc, char ** argv)
{
    MBI nets(argc,argv);
	//MBI nets("./input/example4.paag","./input/example4.sdc","./input/simple-cells.lib");
	nets.max_inv_fanout = 2;
	nets.max_cell_fanout = 2;
	
    nets.set_nodal_delay("AND2_X1","INV_X1");
    //nets.print();
    nets.estimate_delay();
	
    nets.insert_buffers();
    nets.print();
    //nets.lib->print();
}
