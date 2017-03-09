/*!
* \file genome.h
* \brief File containing structures and prototypes for the genome
*/
#ifndef GENOME_H
#define GENOME_H

#include <stdlib.h>
#include <stdio.h>

#include "network.h"
#include "utils.h"

/*!
* \struct Genome genome.h
* \brief A genome contains connection genes
*/
typedef struct {
  /*Network network; */     /*!< the Network attached to this Genome */
} Genome;

Genome * newGenome();
int generateGenome(Genome * genome);

Neuron * getRandomNeuron(Genome * genome);

#endif // GENOME_H
