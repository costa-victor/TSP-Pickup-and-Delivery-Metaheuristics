/*

	Aluno: Victor Alberti Costa
	Problema do Caixeiro Viajante com Coleta e Entrega (TSPPD) de Mosheiov
	Uma abordagem via Variable Neighborhood Search (VNS)

	Todas as instâncias podem ser encontradas em:

	https://hhperez.webs.ull.es/PDsite/#XGLV99

	Na aba Benchmark Instances, procurar por TS2004t2.zip para instâncias entre 20 e 60 entradas.
	Na aba Benchmark Instances, procurar por TS2004t3.zip para instâncias entre 100 e 500 entradas.

	Na aba Solution Values, procurar por TS2004t2.sol e TS2004t3.sol para verificação dos resultados.

	Compilar:
		gcc metaheuristica.c -o m -lm

	Executar:
		time ./m n20mosA.tsp

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>


// Cabeçalhos das funções
// Se vns -> 1.... funcina como vns parando a heuristica no primeiro estado em que melhora
// Se vns -> 0.... funciona como heuristica comum

// Se solInitGuloso -> 1.... retorna para uma variavel para saber que veio do guloso
// Se solInitGuloso -> 0.... retorna em uma variavel para saber que veio de uma sol init. aleatoria
float calc_distance(float *x1, float *y1, float *x2, float *y2);
float swap(int* init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso);
float opt2(int* init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso);
float opt3(int* init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso);
int* getVetorCorrente(int* vet);

// Variavel global para receber a funçao getVetorCorrente.
int *currentSolutionGlobal;

// O mesmo para o alg. guloso
int *currentSolutionGlobalGuloso;

int main (int argc, char *argv[]){
		
	char word[50];
	int flag_dimension=0, flag_capacity=0, flag_matriz = 0, flag_demand = 0;
	int dimension=0, capacity=0, max_capacity=0, commodity_id = 0, pos_col = 0, last_iteration = 0;
	int pickup_demand = 0, delivery_demand = 0;
	int pickup_commodity = 0, delivery_commxodity = 0;
	int end = 0, current_pos = 0, pre_end = 0;
	int one_more = 0, end_point;
	int i=0, j=0, k=0;

    FILE *file;
    file = fopen(argv[1], "r");

    float  optimal_cost = 0, cost = 99999999;
	float **coord;	// armazena a coordenada na leitura do tsp
	float **dist;	// matriz com distancia entre pontos com as coordenadas de coord - SEM DEPOSITO REPETIDO

	typedef struct
    {
    	int demand;
    	int visited;	
    } node;
	node *d;	// armazena as demandas de cada ponto

	int *solution;	// armazena os pontos percorridos em ordem


	// Leitura de arquivo - INICIO  
	if(file == NULL){
		printf("ERRO - nao foi possivel abrir o arquivo\n");
		return 0;
	}
	else{		
		while((fscanf(file, "%s", word))!= EOF){	// while - Leitura de arquivo - INICIO
			/* ====================== leitura de dimension e criação da matriz dinamica - INICIO ======================== */
			if(flag_dimension == 1){		
				dimension = atoi(word);

				// Criação Matriz de coord
				coord = malloc(dimension* sizeof(float*));		// aloca um vetor de tam. dimension para as linhas
				for(i=0; i < dimension; i++){
					coord[i] = malloc(3* sizeof(float));	// aloca um vetor de tam. 3 para as colunas
				}

				// Criação vetor de demanda
				d = malloc(dimension* sizeof(node));	// criacao do vetor para demand e visited

				// Criação do vetor com as respostas
				solution = malloc(dimension* sizeof(int));

				// Criação Matriz de distancias		-- SEM DEPOSITO REPETIDO
				dist = malloc((dimension)* sizeof(float*));		// aloca um vetor de tam. dimension-1 para as linhas
				for(i=0; i < (dimension); i++){
					dist[i] = malloc((dimension)* sizeof(float));	// aloca um vetor de tam. dimension-1 para as colunas
				}

				flag_dimension = 0;

			}
			else if(strcmp(word, "DIMENSION:") == 0){	// palavra chave para ler dimension
				flag_dimension = 1;	 
			}
			/* ====================== leitura de dimension e criação da matriz dinamica - FIM ======================== */


			/* ====================== leitura de capacity - INICIO ======================== */
			if(flag_capacity == 1){	
				max_capacity = atoi(word);
				flag_capacity = 0;

			}
			else if(strcmp(word, "CAPACITY:") == 0){	// palavra chave para ler capacity
				flag_capacity = 1;	 
			}
			/* ====================== leitura de capacity - FIM ======================== */

			/* ====================== leitura das coordenadas - INICIO ======================== */
			if(flag_matriz == 1){		
				if(strcmp(word, "DISPLAY_DATA_SECTION:") == 0){		// muda a flag para sair da leitura de coordenadas
					flag_matriz = 0;
				}
				else if(strcmp(word, "DISPLAY_DATA_SECTION:") != 0){	// palavra chave para fim leitura de coordenadas
						
							coord[i][j] = atof(word);
							if(j == 2){
								i++;
								j=0;
							}
							else{
								j++;
							}

				}

			}
			else if(strcmp(word, "NODE_COORD_SECTION") == 0){	// palavra chave para ler coordenadas
				flag_matriz = 1;
				i=0;	// reseta as variaveis auxiliares
				j=0;					
			}
			/* ====================== leitura das coordenadas - FIM ======================== */

			/* ====================== leitura das demandas - INICIO ======================== */
			if(flag_demand == 1){		

				if(j == 1){
					d[i].demand = atoi(word);
					// Testes
					//printf("%i %i --> %i  -> %i\n", i, j, d[i].demand, d[i].visited);
					i++;
					j=0;
				}
				else{
					j++;
				}

			}
			else if(strcmp(word, "DEMAND_SECTION") == 0){	// palavra chave para ler demandas
				flag_demand = 1;
				i=0;	// reseta as variaveis auxiliares
				j=0;					
			}
			/* ====================== leitura das demandas - FIM ======================== */

		}	// while - Leitura de arquivo - FIM
	}	// Leitura de arquivo - FIM


	/* ====================== Criação da matriz de distancias - INICIO ======================== */
	for(i = 0; i < dimension; i++){
		for(j = 0; j < dimension; j++){
			dist[i][j] = calc_distance(&coord[i][1], &coord[i][2], &coord[j][1], &coord[j][2]);
		}
	}
	/* ====================== Criação da matriz de distancias - FIM ======================== */
	
	
	// Testes
	/*
	printf("\n-------matriz de distancias e controle de visitas------\n");
		for(i = 0; i < dimension; i++){
		for(j = 0; j < dimension; j++){
			printf("%.1f ", dist[i][j]);
			if(j == dimension-1){
				printf("\n");
			}

		}
	}

	printf("\n\n-------matriz de coordenadas------\n");
	for(i = 0; i < dimension; i++){
		for(j = 0; j < 3 ;j++){
			printf("%.3f ", coord[i][j]);
			if(j == 2){
				printf("\n");
			}
		}
	}
	*/
	
	// ALGORITMO DE CALCULO DO CUSTO - GULOSO
	//	------ Primeiro armazena quanto o deposito entrega e quanto recebe ------

	if(d[0].demand >= 0){		
		delivery_demand = d[0].demand;
	}
	else{
		pickup_demand = d[0].demand;
		end_point =  0;
	}
	if(d[dimension-1].demand >= 0){		
		delivery_demand = d[dimension-1].demand;
	}
	else{
		pickup_demand = d[dimension-1].demand;
		end_point = dimension-1;
	}
	
	current_pos = 0;	// inicia da primeira posicao - DEPOSITO - linha 0 da matriz
	capacity = delivery_demand;	// inicia com o que o DEPOSITO fornece
	d[0].visited = 1;
	d[dimension-1].visited = 1;	// identificador do retorno ao DEPOSITO
	// Definindo o start point no vetor de solução
	if(end_point == 0)
		solution[0] = dimension;
	else
		solution[0] = 1;

	// Testes
	/*
	printf("\n\ndelivery demand: %i    --- pickup demand: %i \n\n", delivery_demand, pickup_demand);
	for(i =0; i< dimension; i++){
		printf("%i ", d[i].demand);
	}
	printf("\n");
	*/

	// =============	WHILE - Enquanto nao terminou - INICIO	=================================
	while(end != 1){
		
		if(pre_end == dimension-2){		// se percorreu todos, exceto o DEPOSITO
				

			//current_pos = dimension-1;
			cost = dist[current_pos][end_point];
			current_pos = end_point;
			last_iteration = 1;
			if(d[end_point].demand <= 0){	
				commodity_id = 1;	
			}
			else{
				commodity_id = 2;	
			}
		}

		// ====================== Ve todas as possibilidades a partir do ponto atual e escolhe melhor - INICIO ======================== 
		for(j = 0; j < dimension; j++){
			if((d[j].visited <= 0)){	// caso nao tenha sido visitado e nao tenha sido finalizado

					if( ((d[j].demand + capacity) >= 0) && ((d[j].demand + capacity) <= max_capacity) && (dist[current_pos][j] < cost) && (current_pos != j)){	// percore a linha buscando menor distancia
						cost = dist[current_pos][j];
						if(d[j].demand <= 0){
							commodity_id = 1;	
						}
						else{
							commodity_id = 2;	
						}
						pos_col = j;	// pto de partida do prox. movimento
					}
			}	
				
		}

		// Conclui quem é a melhor possibilidade gulosa
		if(last_iteration == 0){		// Se nao for a ultima iteracao, caso seja o valor dela foi definido acima
			current_pos = pos_col;		// posicao atual alterada			
		}

		d[current_pos].visited = 1;		// marca como visitaprdo
		pre_end++;			// var auxiliar para saber quando falta apenas voltar ao deposito
		optimal_cost = cost + optimal_cost;		//incrementa o calculo do custo
		if(commodity_id == 1){		// tratamento - delivery point
			if(((d[current_pos].demand + capacity) >= 0) && ((d[current_pos].demand + capacity) <= max_capacity)){
				// Testes
				//printf("\ncurrent pos: %i / optimal_cost: %.2f / cost: %.2f ----- %i = %i + (%i)", current_pos ,optimal_cost, cost, (capacity + d[current_pos].demand), capacity, d[current_pos].demand);

				delivery_demand = delivery_demand + d[current_pos].demand;
				capacity = capacity + d[current_pos].demand;	
			}
			pos_col = 0;
			cost = 99999999;
			
		}
		else if(commodity_id == 2){		// tratamento - pickup point			
			if( ((d[current_pos].demand + capacity) >= 0) && ((d[current_pos].demand + capacity) <= max_capacity)){
				// Teste
				//printf("\ncurrent pos: %i / optimal_cost: %.2f / cost: %.2f  ----- %i = %i + (%i)", current_pos, optimal_cost, cost,(capacity + d[current_pos].demand), capacity, d[current_pos].demand);

				pickup_demand = pickup_demand + d[current_pos].demand;	
				capacity = capacity + d[current_pos].demand;				
			}
			// reinicializacao das variaveis
			pos_col = 0;
			cost = 99999999;

		}
		// Incrementação do vetor resposta 
		solution[pre_end] = current_pos+1;
		// Teste
		//printf("\ncapacity: %i = pickup_demand: %i  ----  delivery_demand: %i  --> optimal_cost: %.2f   ===== pre_end: %i\n\n", capacity, pickup_demand, delivery_demand, optimal_cost, pre_end);

		// ====================== Ve todas as possibilidades a partir do ponto atual e escolhe melhor - FIM ======================== 


		if((pickup_demand == 0) && (pickup_demand == 0) && (capacity == 0)){	// se entregou tudo, coletou tudo e ta no deposito == FIM
			//printf("\nGuloso ->  Name: %s 	Cost: %.2f\n\n", argv[1], optimal_cost);	// trecho guloso
			end = 1;
		}

	}
	// =============	WHILE - Enquanto nao terminou - FIM	=================================

	// Preparação do parametro para as heuristicas
	int *dem;
	dem = malloc(dimension* sizeof(int));
	for(int i=0; i < dimension; i++){
		dem[i] = d[i].demand;
	}

	// Resposta gulosa:

	// salvando a resposta gulosa....
	float optimal_cost_guloso = optimal_cost;

	float resSwapG = swap(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 1);
	float res2optG = opt2(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 1);
	float res3optG = opt3(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 1);
	

	// Salvando o vetor de entrada da resposta gulosa
	int* solutionGuloso;
	solutionGuloso = malloc(dimension* sizeof(int));
	for(int i=0; i < dimension; i++)
		solutionGuloso[i] = solution[i];

	// ======================================================================
	// Inicio ----- Geração solução inicial aleatória --------


	// testes
	/*
	printf("\n=========================================================\n");
	printf("Inicio ----- Geração solução inicial aleatória --------\n");
	*/
	int* solutionAleatorio;

	int valido = 0;
	float sortCost = 0;
	int sortCapacity;	
	int repetido = 0;
	int sort;
	int a; // auxiliar	
	srand(time(NULL));

	if(dem[0] > 0)
		pickup_demand = dem[0];
	else
		pickup_demand = dem[dimension-1];

	while(valido != 10){

		solutionAleatorio = malloc(dimension* sizeof(int));
		for(int i=0; i < dimension; i++){
			if(i == 0)
				solutionAleatorio[i] = 1;
			else if(i == dimension-1)
				solutionAleatorio[i] = dimension;
			else
				solutionAleatorio[i] = -6;		// para facilitar ver se tem algo errado
		}
		// testes
		/*
		printf("\nwhile: \n");
		for(int i=0 ; i < dimension; i++)
			printf("%i ", solutionAleatorio[i]);
		printf("\n");
		*/
		valido = 0;
		sortCost = 0;
		repetido = 0;

		if(dem[0] > 0)
			pickup_demand = dem[0];
		else
			pickup_demand = dem[dimension-1];

		sortCapacity = pickup_demand;	
		for(int j = 1; j < dimension-1; j++){			
			do{
				repetido = 0;

				do{
					//srand(time(NULL));
					sort = rand()%dimension;
				}while(sort == 0 || sort == dimension);	// Não pode inverter a primeira Pos. e nem a ultima

				for(int i = 0; i < dimension; i++){
					if(sort == solutionAleatorio[i])
						repetido = 1;	
				}
			}while(repetido == 1);		// Não pode ser repetido
			solutionAleatorio[j] = sort;
		}
		if(dem[0] < 0){	// Correção do inicio...
			a =  solutionAleatorio[dimension-1];
			solutionAleatorio[dimension-1] = solutionAleatorio[0];
			solutionAleatorio[0] = a;
		}

		// Teste
		/*
		printf("\nSolution Aleatorio  -- SortCost: %.2f  --- pickup_demand: %i\n", sortCost, sortCapacity);
		for(int i=0; i < dimension; i++)
			printf("%i\t%i\t%i\n", i, dem[i], solutionAleatorio[i]);
		printf("\n");
		*/
			
		// Calculo do custo e validação
		// Inicialização das variaveis de controle
		for(int i=1; i < dimension; i++){
			sortCapacity = sortCapacity + dem[solutionAleatorio[i]-1];
			//printf("\ndem[%i]: %i  --- capacity: %i", solutionAleatorio[i]-1, dem[solutionAleatorio[i]-1], sortCapacity);
			if(sortCapacity < 0 || sortCapacity > max_capacity){	// validação
				valido = 1;
				// Teste
				//printf("\nMovimento Invalido -- Capacidade: %i -- Max. Permitido: %i", sortCapacity, max_capacity);
				i = dimension-1;
			}
			else
				sortCost = sortCost + dist[solutionAleatorio[i-1]-1][solutionAleatorio[i]-1];
		}
		if(valido == 1){
			//printf("\nRandom invalido\n");
		}
		else if(valido == 0){
			//printf("\nRandom valido   --- sortCost: %.2f\n", sortCost);
			valido = 10;
			optimal_cost = sortCost;
			// Salvando os resultados validos para solução inicial
			for(i = 0; i < dimension; i++)
				solution[i] = solutionAleatorio[i];		
		}
	}

	//printf("\n=========================================================\n");
	
	// Fim ----- Geração solução inicial aleatória --------
	// ======================================================================


	// printando o vetor de resultado
	// SOLUÇAO INICIAL DA HEURISTICA
	/*
	printf("\nSolucao inicial:\n");
	for(i = 0; i < dimension; i++){
		if(i == dimension-1)
			printf("%i", solution[i]);
		else
			printf("%i -> ", solution[i]);
	}
	*/

	// Alocando memoria para passar pra função getVetorCOrrent
	//printf("\ncurrentSolutionGlobal:\n");
	currentSolutionGlobal = malloc(dimension* sizeof(int));
	for(int i=0; i < dimension; i++){	// inciializando o ótimo global corrente como o guloso
		currentSolutionGlobal[i] = solution[i];
		//printf("%i ", currentSolutionGlobal[i]);
	}


	// Geração da resposta para aplicação da metaheuristica VNS com a entrada inciial aleatoria

	// COnjunto Kmax de vizinhanças -----> k=1,2,3 equivale a respectivamente Swap,2opt e 3opt
	// Solução incial do algoritmo guloso -----> solution
	// criterio de parada -----> criterioParada 
	int criterioParada = 0;
	int maxIteracoes = 5;
	k=1;
	float optimal_cost_current = optimal_cost; // melhor custo, assim como a sol. inciial parte da sol. aleatoria
	float costSwap=-1.0, cost2opt=-1.0, cost3opt=-1.0;
	
	// testes
	/*
	printf("\n\nMetaheuristica VNS aleatorio ------ Criterio de parada %i iteracoes\n\n", maxIteracoes);
	printf("Optimal Inicial: %.1f\n", optimal_cost_current);
	for(int i=0; i<dimension; i++)
		printf("%i ", currentSolutionGlobal[i]);
	printf("\n\n\n");	
	*/

	while(criterioParada < maxIteracoes){
		//printf("\n\nIteracao: %i  --  CurrentCost: %.1f\n", criterioParada, optimal_cost_current);
		k = 1;	// começa com swap
		while(k <= 3){
			if(k == 1){
				/*
				printf("\nMain-Swap\n");
				for(int i=0; i<dimension; i++)
					printf("%i ", currentSolutionGlobal[i]);
				printf("\n");
				*/
				costSwap = swap(currentSolutionGlobal, dimension, dem, dist, max_capacity, optimal_cost_current, 1, 0);
						
				if(costSwap < optimal_cost_current){
					k = 1;
					optimal_cost_current = costSwap;
				}
				else{
					k++;
				}
		
			}
			else if(k == 2){
				/*
				printf("\nMain-2opt\n");
				for(int i=0; i<dimension; i++)
					printf("%i ", currentSolutionGlobal[i]);
				printf("\n");
				*/
				cost2opt = opt2(currentSolutionGlobal, dimension, dem, dist, max_capacity, optimal_cost_current, 1, 0);
						
				if(cost2opt < optimal_cost_current){
					k = 1;
					optimal_cost_current = cost2opt;
				}
				else{
					k++;
				}
		
			}
			else if(k == 3){
				//printf("\n\nMain-3opt\n");
				cost3opt = opt3(currentSolutionGlobal, dimension, dem, dist, max_capacity, optimal_cost_current, 1, 0);

				if(cost3opt < optimal_cost_current){
					k = 1;
					optimal_cost_current = cost3opt;
				}
				else
					k++;
			
			}
			/*
			printf("Optimal Current: %.1f\n", optimal_cost_current);
			for(int i=0; i<dimension; i++)
				printf("%i ", currentSolutionGlobal[i]);
			printf("\n");	
			*/
		}

		criterioParada++;
	}


	// Geração da resposta para aplicação da metaheuristica VNS com a entrada do guloso....
	// Alocando memoria para passar pra função getVetorCOrrent 
	//printf("\ncurrentSolutionGlobalGuloso:\n");
	currentSolutionGlobalGuloso = malloc(dimension* sizeof(int));
	for(int i=0; i < dimension; i++){	// inciializando o ótimo global corrente como o guloso
		currentSolutionGlobalGuloso[i] = solutionGuloso[i];
		//printf("%i ", currentSolutionGlobalGuloso[i]);
	}

	//printf("\n\n\n\n\n\n\n\n\n\n");


	criterioParada = 0;

	k=1;
	float optimal_cost_currentG = optimal_cost_guloso; // melhor custo, assim como a sol. inciial parte do guloso
	costSwap=-1.0, cost2opt=-1.0, cost3opt=-1.0;
	
	// testes
	/*
	printf("\n\nMetaheuristica VNS  guloso------ Criterio de parada %i iteracoes\n\n", maxIteracoes);
	printf("Optimal Inicial: %.1f\n", optimal_cost_currentG);
	for(int i=0; i<dimension; i++)
		printf("%i ", currentSolutionGlobalGuloso[i]);
	printf("\n\n\n");	
	*/

	while(criterioParada < maxIteracoes){
		//printf("\n\nIteracao: %i  --  CurrentCost: %.1f\n", criterioParada, optimal_cost_currentG);
		k = 1;	// começa com swap
		while(k <= 3){
			if(k == 1){
				/*
				printf("\nMain-Swap\n");
				for(int i=0; i<dimension; i++)
					printf("%i ", currentSolutionGlobalGuloso[i]);
				printf("\n");
				*/
				costSwap = swap(currentSolutionGlobalGuloso, dimension, dem, dist, max_capacity, optimal_cost_currentG, 1, 1);
						
				if(costSwap < optimal_cost_currentG){
					k = 1;
					optimal_cost_currentG = costSwap;
				}
				else{
					k++;
				}
		
			}
			else if(k == 2){
				/*
				printf("\nMain-2opt\n");
				for(int i=0; i<dimension; i++)
					printf("%i ", currentSolutionGlobalGuloso[i]);
				printf("\n");
				*/
				cost2opt = opt2(currentSolutionGlobalGuloso, dimension, dem, dist, max_capacity, optimal_cost_currentG, 1, 1);
						
				if(cost2opt < optimal_cost_currentG){
					k = 1;
					optimal_cost_currentG = cost2opt;
				}
				else{
					k++;
				}
		
			}
			else if(k == 3){
			
				//printf("\n\nMain-3opt\n");
				cost3opt = opt3(currentSolutionGlobalGuloso, dimension, dem, dist, max_capacity, optimal_cost_currentG, 1, 1);
			
				if(cost3opt < optimal_cost_currentG){
					k = 1;
					optimal_cost_currentG = cost3opt;
				}
				else
					k++;
			
			}
			/*
			printf("Optimal Current: %.1f\n", optimal_cost_current);
			for(int i=0; i<dimension; i++)
				printf("%i ", currentSolutionGlobalGuloso[i]);
			printf("\n");	
			*/
		}

		criterioParada++;
	}

	//armazenando resposta para sol inicial aleatoria de apenas a aplicação das heurisitcas invidualmente
	// opt2(SOLUÇÃO INICIAL, DIMENSAO, DEMANDAS, DISTANCIAS, CAPACIDADE MAXIMA, CUSTO ÓTIMO DA SOLUÇÃO INICIAL)
	float resSwap = swap(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 0);
	float res2opt = opt2(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 0);
	float res3opt = opt3(solution, dimension, dem, dist, max_capacity, optimal_cost, 0, 0);


	printf("\n====================================================================");
	printf("\nTestes para Sol. incial Gulosa com Criterio de parada: %i", maxIteracoes);
	printf("\n====================================================================\n");
	printf("\nGuloso: %.2f\n", optimal_cost_guloso);	
	printf("Swap: %.2f\n2-opt: %.2f\n", resSwapG, res2optG);
	printf("3-opt: %.2f\n", res3optG);
	printf("VNS: %.2f\n", optimal_cost_currentG);	
	//printf("\n");	

	printf("\n====================================================================");
	printf("\nTestes para Sol. incial aleatoria com Criterio de parada: %i", maxIteracoes);
	printf("\n====================================================================\n");
	printf("\nCusto Aleatorio: %.2f\n", optimal_cost);	
	printf("Swap: %.2f\n2-opt: %.2f\n", resSwap, res2opt);
	printf("3-opt: %.2f\n", res3opt);
	printf("VNS: %.2f\n", optimal_cost_current);
	printf("\n");
	// desalocando memória
	free(solution);
	free(coord);
	free(d);
	free(dist);
	free(dem);
	free(currentSolutionGlobal);
	free(currentSolutionGlobalGuloso);
	free(solutionAleatorio);
	free(solutionGuloso);

	return 0;  
}

float swap(int *init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso){
	int sort, aux, pickup_demand, delivery_demand, capacity;
	float optcost=0, optimal_local = gulosoCost;
	int *opt_seq, *swap, *currentSolutionLocalSwap;
	int i = 0;
	int valido = 0;
	int primeiroQueMelhora = 0;


	// Classificando se funcionara como heuristica ou metaheuristica (definido lá emcima)
	if(vns == 1)
		primeiroQueMelhora=0;
	else
		primeiroQueMelhora=10;

	// Vetor para armazenar o melhor local para ser enviado pela função
	//printf("\nSol. INicial-Swap:\n");
	currentSolutionLocalSwap = malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		currentSolutionLocalSwap[i] = init[i];
		//printf("%i ", currentSolutionLocalSwap[i]);
	}

	// Vetor auxiliar para sequencia
	opt_seq = malloc((dim-3)* sizeof(int));

	// swap recebe sol. inicial, que veio do guloso
	swap =  malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		swap[i] = init[i];
	}

	// Escolhe uma posicao aleatoriamente, para fazer a troca com o elem. da sua direita e armazena num vetor Ex: sort = 1--- 1-2
	int repetido = 0;
	for(int j = 0; j < dim-3; j++){			
		do{
			repetido = 0;

			do{
				sort = rand()%dim;
			}while(sort == 0 || sort == dim-1 || sort == dim-2);	// Não pode inverter a primeira Pos. e nem a ultima

			for(int i = 0; i < dim-3; i++){
				if(sort == opt_seq[i])
					repetido = 1;	
			}
		}while(repetido == 1);		// Não pode ser repetido
		opt_seq[j] = sort;
	}

	// Testes
	/*
	printf("\n");
	printf("\nSeq. de Swap na Sol. Inicial:\n");
	for(int i=0; i<dim-3; i++)
		printf("%i ", opt_seq[i]+1);
	printf("\n");
	*/

	// INICIO -------> BUSCA LOCAL <--------
	for(int j=0; j < dim-3; j++){
		i = 0;
		optcost = 0;
		valido = 0;
		sort = opt_seq[j];

		// OPt2 recebe sol. inicial
		for(int i=0; i < dim; i++){
			swap[i] = init[i];
		}

		// Troca 2-opt
		aux = swap[sort+1];		// recebe elem. da direita
		swap[sort+1] = swap[sort];
		swap[sort] = aux;

		// testes
		/*	
		printf("\nNum trocado: %i\n", sort+1);
		
		for(int i =0; i< dim; i++){
			printf("%i -> ",swap[i]);
		}
		printf("\nDemandas\n");
		for(int i =0; i< dim; i++){
			printf("%i -> ", i);
		}
		printf("\n");
		for(int i =0; i< dim; i++){
			printf("%i -> ",dem[i]);
		}
		//*/

		// Demandas do veiculo
		if(dem[0] > 0)
			delivery_demand = dem[0];		
		else
			pickup_demand = dem[0];

		if(dem[dim-1] > 0)
			delivery_demand = dem[dim-1];	// Qtde positiva - para entregar - Sai do deposito
		else
			pickup_demand = dem[dim-1];		// Qtde negativa - para coletar - Retorna ao deposito

		capacity = delivery_demand;		// capacidade inicial do veiculo
		//printf("\n\nQtde para delivery: %i --- Qtde para pickup: %i\n", delivery_demand, pickup_demand);	// testes

		// Calculo do custo e validação
		while(i < dim-1){	
			if(dem[swap[i+1]-1] > 0)
				pickup_demand = pickup_demand + dem[swap[i+1]-1];
			else
				delivery_demand = delivery_demand + dem[swap[i+1]-1];

			capacity = capacity + dem[swap[i+1]-1];
			//printf("capacity %i : %i\n", swap[i+1]-1, capacity);	// testes
			if(capacity < 0 || capacity > max){		// validação
				valido = 1;
				//printf("\nMovimento Invalido!! - Troca entre %i <-> %i", swap[sort], swap[sort+1]);
				break;
			}
			optcost = optcost + dist[swap[i]-1][swap[i+1]-1];	// Calculo custo
			i++;
		}
		delivery_demand = delivery_demand + max;
		//printf("\n\nQtde para delivery: %i --- Qtde para pickup: %i\n", delivery_demand, pickup_demand);	// testes 
		if(valido == 0){	// Se foi um movimento válido
			//printf("\nTroca entre %i <-> %i -- Custo: %.1f", swap[sort], swap[sort+1], optcost); 
			if(optcost < optimal_local){
				if(primeiroQueMelhora == 0){
					optimal_local = optcost;

					for(int i=0; i < dim; i++){
						currentSolutionLocalSwap[i] = swap[i];
					}

					if(solInitGuloso == 1)
						currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocalSwap);
					else if(solInitGuloso == 0)	
						currentSolutionGlobal = getVetorCorrente(currentSolutionLocalSwap);
					// testes
					/*
					printf("\n");
					for(int i=0; i < dim; i++){
						printf("%i ", currentSolutionGlobal[i]);
					}
					printf("\nCurrentCost: %.1f - Current-Swap\n", optimal_local);
					printf("\n\n");	
					*/
					primeiroQueMelhora = 1;				
				}
				else{
					optimal_local = optcost;

					for(int i=0; i < dim; i++){
						currentSolutionLocalSwap[i] = swap[i];
					}
					if(solInitGuloso == 1)
						currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocalSwap);
					else if(solInitGuloso == 0)	
						currentSolutionGlobal = getVetorCorrente(currentSolutionLocalSwap);


					// testes
					/*		
					printf("\n");
					for(int i=0; i < dim; i++){
						printf("%i ", currentSolutionGlobal[i]);
					}
					printf("\nCurrentCost: %.1f - Current-Swap\n", optimal_local);
					printf("\n\n");	
					*/					
				}

			}
		}
	}		// FIM -------> BUSCA LOCAL <--------


	// Desalocando memória
	//free(opt_seq);
	//free(swap);
	//free(currentSolutionLocalSwap);

	// Retorna o melhor resultado encontrado
	return optimal_local;
}

// opt2(SOLUÇÃO INICIAL, DIMENSAO, DEMANDAS, DISTANCIAS, CAPACIDADE MAXIMA, CUSTO ÓTIMO DA SOLUÇÃO INICIAL)
float opt2(int *init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso){
	//printf("\n\n2-opt:\n\n");
	int iteracoes = 0;
	int primeiroQueMelhora=0;

	int sort, aux, pickup_demand, delivery_demand, capacity;
	float opt2cost=0, optimal_local = gulosoCost;
	int *opt2_new, *opt2, *currentSolutionLocal2opt;
	int valido = 0;
	int p1[2];
	int p2[2];
	int i=0;

	// Classificando se funcionara como heuristica ou metaheuristica (definido lá emcima)
	if(vns == 1)
		primeiroQueMelhora=0;
	else
		primeiroQueMelhora=10;

	// Vetor para armazenar o melhor local para ser enviado pela função
	//printf("\nSol. INicial-2opt:\n");
	currentSolutionLocal2opt = malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		currentSolutionLocal2opt[i] = init[i];
		//printf("%i ", currentSolutionLocal2opt[i]);
	}

	// Vetor auxiliar para a nova sequencia
	
	opt2_new = malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		opt2_new[i] = init[i];
		
	}

	// opt2 recebe sol. inicial, que veio do guloso
	opt2 =  malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		opt2[i] = init[i];
	}

	// Demandas do veiculo
	if(dem[0] > 0)
		delivery_demand = dem[0];		
	else
		pickup_demand = dem[0];

	if(dem[dim-1] > 0)
		delivery_demand = dem[dim-1];	// Qtde positiva - para entregar - Sai do deposito
	else
		pickup_demand = dem[dim-1];		// Qtde negativa - para coletar - Retorna ao deposito

	capacity = delivery_demand;		// capacidade inicial do veiculo
	//printf("\n\nQtde para delivery: %i --- Qtde para pickup: %i\n", delivery_demand, pickup_demand);	// testes

	// Escolhe uma posicao aleatoriamente, para fazer a troca com o elem. da sua direita e armazena num vetor Ex: sort = 1--- 1-2
	// Ser exaustivo e ver par a par, desconsiderando o ultimo ponto que é o mesmo que o primeiro
	for(int i=0; i < dim-1; i++){
		for(int j=0; j < dim; j++){
			iteracoes++;
			// Seleciona um par para ver todas as comb. dele
			p1[0]=opt2[i];	
			p1[1]=opt2[i+1];

			// Definindo o segundo par
			if(j+1 <= dim-1){	// se o segundo elemento no par pertencer ao vetor...
				// Garante iniciar sempre do mesmo vetor incial
				for(int i=0; i < dim; i++){
					opt2_new[i] = opt2[i];
				}
				if((opt2[j] != p1[0] && opt2[j] != p1[1]) && (opt2[j+1] != p1[0] && opt2[j+1] != p1[1])){	// e tambem se não pertencer ao par1
					// Se respeita isso, é um indice valido do vetor,basta verificar o custo e ver se é valido a troca...
					p2[0]=opt2[j];
					p2[1]=opt2[j+1];

					// Teste
					//printf("\n\nANTES---\nPar1:[%i][%i]\nPar2:[%i][%i]\n", p1[0], p1[1], p2[0], p2[1]);
					// Realiza a troca - Só tem uma troca possivel, conforme a pag24:
					// https://paginas.fe.up.pt/~mac/ensino/docs/OR/CombinatorialOptimizationHeuristicsLocalSearch.pdf
					// Porem só não realiza a troca quando não deve
					aux = p2[0];
					p2[0] = p1[1];
					p1[1] = aux;

					// comparando o p2[0], pois agr ele é o p1[1]
					if(p2[0] == init[dim-1]){	// Se estiver no ultimo par a troca não deve ocorrer da msm forma
						// inverte os indices certos, para não mudar o deposito de lugar
						aux = p1[0];
						p1[0] = p2[1];
						p2[1] = aux;
						// Repara o que foi feito antes do if...
						aux = p1[1];
						p1[1] = p2[0];
						p2[0] = aux;
						// Atualiza na nova posição
						opt2_new[i] = p1[0];
						opt2_new[j+1] = p2[1];

					}
					else{
						opt2_new[i+1] = p1[1];
						opt2_new[j] = p2[0];	
					}
					
					// Testes
					/*
					printf("DEPOIS---\nPar1:[%i][%i]\nPar2:[%i][%i]\n", p1[0], p1[1], p2[0], p2[1]);
					printf("Vetor inicial:\n");
					for(int i=0; i < dim;i++){
						printf("%i > ", init[i]);	
					}
					printf("\nNovo vetor:");					
					printf("\n");		
					for(int i=0; i < dim; i++)
						printf("%i   ", i);
					printf("\n");			
					for(int i=0; i < dim;i++){
						printf("%i > ", opt2_new[i]);	
					}
					*/
					
					// Calculo do custo e validação
					// Inicialização das variaveis de controle
					valido = 0;
					opt2cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt2_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("\nMovimento Invalido -- Capacidade: %i -- Max. Permitido: %i", capacity, max);
							i = dim-1;
						}
						else
							opt2cost = opt2cost + dist[opt2_new[i]-1][opt2_new[i+1]-1];
					}
					if(valido == 0){
						// Teste
						//printf("\nTroca valida - Capacidade: %i - Custo: %.1f",capacity, opt2cost);
						if(opt2cost < optimal_local){
							// Mudança para armazenar e retornar o primeiro que melhora e não verificar entre todos....
							if(primeiroQueMelhora == 0){	// vns
								optimal_local = opt2cost;

								for(int i=0; i < dim; i++){
									currentSolutionLocal2opt[i] = opt2_new[i];
								}
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal2opt);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal2opt);								
								
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal2opt);
								// testes
								/*					
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nCurrentCost: %.1f - Current-2opt\n", optimal_local);
								printf("\n\n");
								*/
								primeiroQueMelhora = 1;								
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								optimal_local = opt2cost;

								for(int i=0; i < dim; i++){
									currentSolutionLocal2opt[i] = opt2_new[i];
								}
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal2opt);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal2opt);								
								
								
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal2opt);
								// testes
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nCurrentCost: %.1f - Current-2opt\n", optimal_local);
								printf("\n\n");
								*/
							}
						}
					}
		
				}	// FIM - SE SEGUNDO PAR É VALIDO
			}	// FIM - DEFINE O SEGUNDO PAR.
			else{
				// Testes
				/*
				printf("\n\nPar invalido - \n\n");
				for(int i=0; i < dim; i++){
					for(int j=0; j < dim; j++){
						printf("%1.f - ", dist[i][j]);
						if(j == dim-1)
							printf("\n");
					}
				}
				printf("\ncapacity: %i  --- max: %i\n\n", capacity, max);
				for(int i=0; i < dim; i++)
					printf("%i\t", i);
				printf("\n");
				for(int i=0; i < dim; i++)
					printf("%i\t", dem[i]);
				*/
			}
		}
	}
	
	//printf("Qtde de iteracoes: %i\n", iteracoes);


	// Desalocando memória
	//free(opt2);
	//free(opt2_new);
	//free(currentSolutionLocal2opt);

	// Retorna o melhor resultado encontrado
	return optimal_local;
}


// opt3(SOLUÇÃO INICIAL, DIMENSAO, DEMANDAS, DISTANCIAS, CAPACIDADE MAXIMA, CUSTO ÓTIMO DA SOLUÇÃO INICIAL)
float opt3(int *init, int dim, int *dem, float **dist, int max, float gulosoCost, int vns, int solInitGuloso){
	//printf("\n\n3-opt:\n\n");
	int iteracoes = 0;
	int sort, aux, aux1, aux2, aux3, aux4, pickup_demand, delivery_demand, capacity;
	float opt3cost = 0, optimal_local = gulosoCost;
	int *opt3_new, *opt3, *currentSolutionLocal;
	int valido = 0;
	int p1[2];
	int p2[2];
	int p3[2];
	int i=0;
	int move = 1;
	int index1, index2, index3, index4;
	int primeiroQueMelhora=0;
	// Classificando se funcionara como heuristica ou metaheuristica (definido lá emcima)
	if(vns == 1)
		primeiroQueMelhora=0;
	else
		primeiroQueMelhora=10;

	// Vetor para armazenar o melhor local para ser enviado pela função
	//printf("Sol. INicial-3opt:\n");
	currentSolutionLocal = malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		currentSolutionLocal[i] = init[i];
		//printf("%i ", currentSolutionLocal[i]);
	}

	// Vetor auxiliar para a nova sequencia
	opt3_new = malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		opt3_new[i] = init[i];
	}

	// opt2 recebe sol. inicial, que veio do guloso
	opt3 =  malloc(dim* sizeof(int));
	for(int i=0; i < dim; i++){
		opt3[i] = init[i];
	}

	// Demandas do veiculo
	if(dem[0] > 0)
		delivery_demand = dem[0];		
	else
		pickup_demand = dem[0];

	if(dem[dim-1] > 0)
		delivery_demand = dem[dim-1];	// Qtde positiva - para entregar - Sai do deposito
	else
		pickup_demand = dem[dim-1];		// Qtde negativa - para coletar - Retorna ao deposito

	capacity = delivery_demand;		// capacidade inicial do veiculo

	// Escolhe uma posicao aleatoriamente, para fazer a troca com o elem. da sua direita e armazena num vetor Ex: sort = 1--- 1-2
	// Ser exaustivo e ver par a par, desconsiderando o ultimo ponto que é o mesmo que o primeiro
	int j = 0, k = 0;
	for(int i=0; i < dim-5; i++){ 		// PRIMEIRO LAÇO
		iteracoes++;
		for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
			opt3_new[i] = init[i];
		}
		p1[0] = opt3[i];
		p1[1] = opt3[i+1];
		j = (i+1)+1;
		for(j; j < dim-3; j++){		// SEGUNDO LAÇO
			iteracoes++;
			for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
				opt3_new[i] = init[i];
			}
			p2[0] = opt3[j];
			p2[1] = opt3[j+1];
			k = (j+1)+1;
			for(k; k < dim-1; k++){		// TERCEIRO LAÇO 
				iteracoes++;
				for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
					opt3_new[i] = init[i];
				}
				// Definido quem será o ponto 3
				p3[0] = opt3[k];
				p3[1] = opt3[k+1];
				// Teste
				/*
				printf("\nANTES:\n");
				for(int i=0; i < dim; i++)
					printf("%i - ", opt3[i]);
				printf("\nPar1:[%i][%i]\nPar2:[%i][%i]\nPar3:[%i][%i]\n", p1[0], p1[1], p2[0], p2[1], p3[0], p3[1]);
				*/

				move = 1;
				if(move == 1){	// primeiro movimento
					//printf("\nMovimento 1\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 1
					aux = p2[0];
					p2[0] = p1[1];
					p1[1] = aux;

					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p1[1])
							index1 = i;
						if(opt3_new[i] == p2[0])
							index2 = i;
					}
					if(index1 != -1 && index2 != -1){
						aux = opt3_new[index1];
						opt3_new[index1] = opt3_new[index2];
						opt3_new[index2] = aux;
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [1]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/
					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("\nMovimento Invalido -- Capacidade: %i -- Max. Permitido: %i", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){	// vns
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;

								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
								

								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM1 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
								primeiroQueMelhora = 1;							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								

								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM1 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}
						}
					}

					move = 2;	// incrementa para possibilitar a outra verificação
				}
				if(move == 2){	// movimento 2
					//printf("\nMovimento 2\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 2
					aux = p3[0];
					p3[0] = p1[1];
					p1[1] = aux;

					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p1[1])
							index1 = i;
						if(opt3_new[i] == p3[0])
							index2 = i;
					}
					if(index1 != -1 && index2 != -1){
						aux = opt3_new[index1];
						opt3_new[index1] = opt3_new[index2];
						opt3_new[index2] = aux;
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [2]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/
					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("\nMovimento Invalido -- Capacidade: %i -- Max. Permitido: %i", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM2 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM2 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");
								*/	
							}							
						}
					}
					move = 3;	// incrementa para possibilitar a outra verificação
				}
				if(move == 3){	// movimento 3
					//printf("\nMovimento 3\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 3
					aux = p3[0];
					p3[0] = p2[1];
					p2[1] = aux;

					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p2[1])
							index1 = i;
						if(opt3_new[i] == p3[0])
							index2 = i;
					}
					if(index1 != -1 && index2 != -1){
						aux = opt3_new[index1];
						opt3_new[index1] = opt3_new[index2];
						opt3_new[index2] = aux;
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [3]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/
					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("\nMovimento Invalido -- Capacidade: %i -- Max. Permitido: %i", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								

								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM3 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;

								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM3 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}							
						}
					}

					move = 4;	// incrementa para possibilitar a outra verificação
				}
				if(move == 4){	// movimento 4
					//printf("\nMovimento 4\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 4
					aux = p1[1];
					aux1 = p2[0];
					aux2 = p2[1];
					aux3 = p3[0];
					p3[0] = aux;
					p2[0] = aux3;
					p2[1] = aux1;
					p1[1] = aux2;

					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					index3 = -1;
					index4 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p3[0])
							index1 = i;
						if(opt3_new[i] == p2[0])
							index2 = i;
						if(opt3_new[i] == p2[1])
							index3 = i;
						if(opt3_new[i] == p1[1])
							index4 = i;
					}
					if(index1 != -1 && index2 != -1 && index3 != -1 && index4 != -1){
						aux = opt3_new[index1];
						aux1 = opt3_new[index2];
						aux2 = opt3_new[index3];
						aux3 = opt3_new[index4];
						
						opt3_new[index1] = aux3;
						opt3_new[index2] = aux;
						opt3_new[index3] = aux1;
						opt3_new[index4] = aux2;

						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [4]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/
					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("Movimento Invalido -- Capacidade: %i -- Max. Permitido: %i\n", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM4 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
								
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM4 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}							
						}
					}
					move = 5;	// incrementa para possibilitar a outra verificação
				}
				if(move == 5){	// movimento 5
					//printf("\nMovimento 5\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 5
					aux = p2[0];
					aux1 = p3[0];
					p2[0] = p1[1];
					p1[1] = aux;
					p3[0] = p2[1];
					p2[1] = aux1;
					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					index3 = -1;
					index4 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p1[1])
							index1 = i;
						if(opt3_new[i] == p2[0])
							index2 = i;
						if(opt3_new[i] == p2[1])
							index3 = i;
						if(opt3_new[i] == p3[0])
							index4 = i;
					}
					if(index1 != -1 && index2 != -1 && index3 != -1 && index4 != -1){
						aux = opt3_new[index1];
						aux1 = opt3_new[index2];
						aux2 = opt3_new[index3];
						aux3 = opt3_new[index4];
						opt3_new[index1] = aux1;
						opt3_new[index2] = aux;
						opt3_new[index3] = aux3;
						opt3_new[index4] = aux2;	
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [5]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/

					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("Movimento Invalido -- Capacidade: %i -- Max. Permitido: %i\n", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
								
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM5 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM5 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}							
						}
					}

					move = 6;	// incrementa para possibilitar a outra verificação
				}
				if(move == 6){	// movimento 6
					//printf("\nMovimento 6\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 6
					aux = p1[1];
					aux1 = p2[0];
					aux2 = p2[1];
					aux3 = p3[0];
					p2[1] = aux;
					p2[0] = aux2;
					p3[0] = aux1;
					p1[1] = aux3;
					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					index3 = -1;
					index4 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p1[1])
							index1 = i;
						if(opt3_new[i] == p2[0])
							index2 = i;
						if(opt3_new[i] == p2[1])
							index3 = i;
						if(opt3_new[i] == p3[0])
							index4 = i;
					}
					if(index1 != -1 && index2 != -1 && index3 != -1 && index4 != -1){
						aux = opt3_new[index3];
						opt3_new[index3] = opt3_new[index2];
						opt3_new[index2] = aux;
						aux1 = opt3_new[index4];
						opt3_new[index4] = opt3_new[index3];
						opt3_new[index3] = aux1;
						aux2 = opt3_new[index1];
						opt3_new[index1] = opt3_new[index3];
						opt3_new[index3] = aux2;
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [6]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/

					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("Movimento Invalido -- Capacidade: %i -- Max. Permitido: %i\n", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM6 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM6 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}							
						}
					}

					move = 7;	// incrementa para possibilitar a outra verificação
				}				
				if(move == 7){
					//printf("\nMovimento 7\n");
					for(int i=0; i < dim; i++){		// reseta os dados a serem comparados
						opt3_new[i] = init[i];
					}
					p1[0] = opt3[i];
					p1[1] = opt3[i+1];
					p2[0] = opt3[j];
					p2[1] = opt3[j+1];
					p3[0] = opt3[k];
					p3[1] = opt3[k+1];
					// Troca do movimento 7
					aux = p1[1];
					aux1 = p2[0];
					aux2 = p2[1];
					aux3 = p3[0];
					p2[1] = aux;
					p1[1] = aux2;
					p2[0] = aux3;
					p3[0] = aux1;
					
					// Alterando isso no vetor
					index1 = -1;
					index2 = -1;
					index3 = -1;
					index4 = -1;
					for(int i=0; i < dim; i++){
						if(opt3_new[i] == p1[1])
							index1 = i;
						if(opt3_new[i] == p2[0])
							index2 = i;
						if(opt3_new[i] == p2[1])
							index3 = i;
						if(opt3_new[i] == p3[0])
							index4 = i;
					}
					if(index1 != -1 && index2 != -1 && index3 != -1 && index4 != -1){
						aux = opt3_new[index3];
						opt3_new[index3] = opt3_new[index1];
						opt3_new[index1] = aux;
						aux2 = opt3_new[index2];
						opt3_new[index2] = opt3_new[index4];
						opt3_new[index4] = aux2;
						//printf("Troca de %i por %i\n", opt3_new[index1], opt3_new[index2]);
					}
					else
						printf("\nErro [7]\n");
					// Teste
					/*
					printf("DEPOIS:\n");
					for(int i=0; i < dim; i++)
						printf("%i - ", opt3_new[i]);
					printf("\n");
					*/
					// Calculando a distancia e validade a partir do vetor
					valido = 0;
					opt3cost = 0;
					capacity = delivery_demand;

					for(int i=0; i < dim; i++){
						capacity = capacity + dem[opt3_new[i+1]-1];
						if(capacity < 0 || capacity > max){	// validação
							valido = 1;
							// Teste
							//printf("Movimento Invalido -- Capacidade: %i -- Max. Permitido: %i\n", capacity, max);
							i = dim-1;
						}
						else
							opt3cost = opt3cost + dist[opt3_new[i]-1][opt3_new[i+1]-1];
					}
					if(valido == 0 && capacity == 0){
						// Teste
						//printf("Troca valida - Capacidade: %i - Custo: %.1f\n",capacity, opt3cost);
						if(opt3cost < optimal_local){
							// Se realmente a solução melhorar....
							if(primeiroQueMelhora == 0){
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM7 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/							
							}
							else if(primeiroQueMelhora == 10){	// heuristica
								for(int i=0; i < dim; i++){
									currentSolutionLocal[i] = opt3_new[i];
								}
								optimal_local = opt3cost;
								if(solInitGuloso == 1)
									currentSolutionGlobalGuloso = getVetorCorrente(currentSolutionLocal);
								else if(solInitGuloso == 0)	
									currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);								
																
								//currentSolutionGlobal = getVetorCorrente(currentSolutionLocal);
								//
								/*
								printf("\n");
								for(int i=0; i < dim; i++){
									printf("%i ", currentSolutionGlobal[i]);
								}
								printf("\nM7 -> CurrentCost: %.1f - Current-3opt\n", optimal_local);
								printf("\n\n");	
								*/
							}							
						}
					}

					move = 1;	// inicia a contagem dos 7 tipos de movimentos possiveis
				}

			}
		}	
					
	}

	//printf("Quantidade de iteracoes: %i\n", iteracoes);

	// Desalocando memória
	//free(opt3);
	//free(opt3_new);
	//free(currentSolutionLocal);

	// Retorna o melhor resultado encontrado
	return optimal_local;
}
		
int* getVetorCorrente(int* vet){
	return vet;
}	

float calc_distance(float *x1, float *y1, float *x2, float *y2){
	//printf("x1 = %.2f, y1 = %.2f, x2 = %.2f, y2 = %.2f  ---> %.2f\n", x1, y1, x2, y2, (sqrt( pow( (x2-x1) , 2) + pow( (y2-y1) , 2) )));
	//printf("x1 = %.2f, y1 = %.2f, x2 = %.2f, y2 = %.2f  --> dist: %.2f\n", *x1, *y1, *x2, *y2);
	return (sqrt( ((*x2)-(*x1))*((*x2)-(*x1)) + ((*y2)-(*y1))*((*y2)-(*y1)) ));

}

// compile com gcc guloso.c -o gula -lm
// time ./gula n20mosA.tsp   ------- se quiser ver o tempo

