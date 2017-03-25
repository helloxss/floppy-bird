#include "genome.h"

/*!
* \brief Generate a Genome by creating its network elements
* \param[out] genome the Genome to generate
* \return Return 1 if the Genome was successfully generated, 0 otherwise
*/
int generateGenome(Genome * genome) {
  int i;

  for (i = 0; i < N_INPUTS; ++i) {
    if (!addNeuronToNetwork(genome->network, newNeuron(INPUT)))
      return 0;
  }
  for (i = 0; i < N_BIAS; ++i) {
    if (!addNeuronToNetwork(genome->network, newNeuron(BIAS)))
      return 0;
  }
  for (i = 0; i < N_OUTPUTS; ++i) {
    if (!addNeuronToNetwork(genome->network, newNeuron(OUTPUT)))
      return 0;
  }

  return 1;
}

/*!
* \brief Randomly update the weight of a randomly selected ConnectionGene from the given Genome
* \param[out] genome The Genome whose a random ConnectionGene has to be updated
* \return int 1 if a ConnectionGene was successfully updated, 0 otherwise
*
* The updates are either:
* New Weight = Old Weight +/- Random number between 0 and pointMutationRate
* or
* New Weight = Random number between -2 and 2
*/
int mutatePoint(Genome * genome) {
  Neuron * current_neuron = NULL;
  ConnectionGene * current_connection_gene = NULL;

  setOnFirstNeuron(genome->network);
  while (!outOfNeuronList(genome->network)) {

    current_neuron = getCurrentNeuron(genome->network);

    if (current_neuron == NULL)
      return 0;

    setOnFirstConnectionGene(current_neuron->connections);
    while (!outOfConnectionGeneList(current_neuron->connections)) {

      current_connection_gene = getCurrentConnectionGene(current_neuron->connections);

      if (current_connection_gene == NULL)
        return 0;

      if (random01() < POINT_MUTATION_PERTURBATION)
        current_connection_gene->weight += 2.0 * random01() * POINT_MUTATION_RATE - POINT_MUTATION_RATE;
      else
        current_connection_gene->weight = 4.0 * random01() - 2.0;

      nextConnectionGene(current_neuron->connections);
    }

    nextNeuron(genome->network);
  }

  return 1;
}

/*!
* \brief Randomly enable and disable ConnectionGene elements from the given Genome
* \param[out] genome the Genome whose ConnectionGene elements have to be updated
* \param[in] enable
* \return int 1 if the ConnectionGene elements were successfully updated, 0 otherwise
*/
int mutateEnableFlag(Genome * genome, unsigned char enable) {
  int i = 0;
  int random_connection_gene_index;
  Neuron * current_neuron = NULL;
  ConnectionGene * current_connection_gene = NULL;
  ConnectionGene * candidates[N_MAX_NEURONS * N_MAX_CONNECTION_GENES];

  setOnFirstNeuron(genome->network);
  while (!outOfNeuronList(genome->network)) {

    current_neuron = getCurrentNeuron(genome->network);

    if (current_neuron == NULL)
      return 0;

    setOnFirstConnectionGene(current_neuron->connections);
    while (!outOfConnectionGeneList(current_neuron->connections)) {

      current_connection_gene = getCurrentConnectionGene(current_neuron->connections);

      if (current_connection_gene == NULL)
        return 0;

      if (current_connection_gene->enabled != enable) {
        candidates[i] = current_connection_gene;
        ++i;
      }

      nextConnectionGene(current_neuron->connections);
    }

    nextNeuron(genome->network);
  }

  if (i == 0) {
    fprintf(stderr, "There is no ConnectionGene candidate for enable flag mutation\n");
    return 0;
  }

  random_connection_gene_index = randomAtMost(i);
  candidates[random_connection_gene_index]->enabled = !candidates[random_connection_gene_index]->enabled;

  return 1;
}

/*!
* \brief
* \param[out] genome
* \return int 1 if the mutation was successful, 0 otherwise
*
* This mutation adds a new Neuron to the network by disabling a ConnectionGene,
* replacing it with a ConnectionGene of weight 1, a Neuron and a ConnectionGene with
* the same weight as the disabled ConnectionGene.
*
* In essence it’s been replaced with an identically functioning equivalent.
*/
int mutateNode(Genome * genome) {
  int i = 0;
  int random_connection_gene_index;
  Neuron * current_neuron = NULL;
  ConnectionGene * current_connection_gene = NULL;
  ConnectionGene * candidates[N_MAX_NEURONS * N_MAX_CONNECTION_GENES];

  Neuron * new_neuron = NULL;
  ConnectionGene * new_connection_gene_1 = NULL;
  ConnectionGene * new_connection_gene_2 = NULL;

  setOnFirstNeuron(genome->network);
  while (!outOfNeuronList(genome->network)) {

    current_neuron = getCurrentNeuron(genome->network);

    if (current_neuron == NULL)
      return 0;

    setOnFirstConnectionGene(current_neuron->connections);
    while (!outOfConnectionGeneList(current_neuron->connections)) {

      current_connection_gene = getCurrentConnectionGene(current_neuron->connections);

      if (current_connection_gene == NULL)
        return 0;

      candidates[i] = current_connection_gene;
      ++i;

      nextConnectionGene(current_neuron->connections);
    }

    nextNeuron(genome->network);
  }

  if (i == 0) {
    fprintf(stderr, "There is no ConnectionGene candidate for node mutation\n");
    return 0;
  }

  random_connection_gene_index = randomAtMost(i);

  if (candidates[random_connection_gene_index]->enabled == 0)
    return 0;

  candidates[random_connection_gene_index]->enabled = 0;

  new_neuron = newNeuron(BASIC);
  if (!addNeuronToNetwork(genome->network, new_neuron))
    return 0;

  new_connection_gene_1 = cloneConnectionGene(candidates[random_connection_gene_index]);
  new_connection_gene_1->weight = 1.0;
  new_connection_gene_1->enabled = 1;
  addConnectionGeneToNeurons(new_connection_gene_1->neuron_in, new_neuron, new_connection_gene_1);

  new_connection_gene_2 = cloneConnectionGene(candidates[random_connection_gene_index]);
  new_connection_gene_2->enabled = 1;
  addConnectionGeneToNeurons(new_neuron, new_connection_gene_2->neuron_out, new_connection_gene_2);

  return 1;
}

/*!
* \brief Return a random Neuron from the given Genome.
* \param[in] genome the Genome to choose a Neuron from
* \return Return a random Neuron
*/
Neuron * getRandomNeuron(Genome * genome) {
  setOnNeuron(genome->network, randomAtMost(countNeurons(genome->network) - 1));
  return getCurrentNeuron(genome->network);
}

/*!
* \brief Write a the Network of a Genome to an output file, based on graphviz.
* \param[in] genome the Genome to write
* \param[in] filename the name of the output file
* \return int 1 if the file was successfully written, 0 otherwise
*/
int writeGraphVizGenome(Genome * genome, char * filename) {
  FILE * f = NULL;
  ConnectionGeneList * connection_gene_successors = NULL;

  if ((f = (FILE *) fopen(filename, "w")) == (FILE *) NULL) {
      fprintf(stderr, "Error while opening %s\n", filename);
      return 0;
  }

  fprintf(f, "digraph {\n");

  // nodes

  setOnFirstNeuron(genome->network);
  while (!outOfNeuronList(genome->network)) {
      connection_gene_successors = genome->network->current->connections;
      setOnFirstConnectionGene(connection_gene_successors);

      if (emptyConnectionGeneList(connection_gene_successors))
        fprintf(f, "\t%d;\n", genome->network->current->id);

      else {
        while (!outOfConnectionGeneList(connection_gene_successors)) {
          if (connection_gene_successors->current->enabled)
            fprintf(f, "\t%d -> %d [label=\"%.1f\", weight=%.1f];\n", genome->network->current->id, connection_gene_successors->current->neuron_out->id, connection_gene_successors->current->weight, connection_gene_successors->current->weight);
          else
            fprintf(f, "\t%d -> %d [label=\"%.1f\", weight=%.1f color=red];\n", genome->network->current->id, connection_gene_successors->current->neuron_out->id, connection_gene_successors->current->weight, connection_gene_successors->current->weight);

          nextConnectionGene(connection_gene_successors);
        }
      }

      nextNeuron(genome->network);
  }

  // colors

  setOnFirstNeuron(genome->network);
  while(!outOfNeuronList(genome->network)) {
    if (genome->network->current->type == INPUT)
      fprintf(f, "\t%d [label=\"%d\\n%.1f\", shape=circle, style=filled, fillcolor=yellow];\n", genome->network->current->id, genome->network->current->id, genome->network->current->value);

    else if (genome->network->current->type == OUTPUT)
      fprintf(f, "\t%d [label=\"%d\\n%.1f\", shape=circle, style=filled, fillcolor=red];\n", genome->network->current->id, genome->network->current->id, genome->network->current->value);

    else if (genome->network->current->type == BIAS)
      fprintf(f, "\t%d [label=\"%d\\n%.1f\", shape=circle, style=filled, fillcolor=orange];\n", genome->network->current->id, genome->network->current->id, genome->network->current->value);

    else if (genome->network->current->type == BASIC)
      fprintf(f, "\t%d [label=\"%d\\n%.1f\", shape=circle];\n", genome->network->current->id, genome->network->current->id, genome->network->current->value);

    else
      fprintf(f, "\t%d [shape=circle];\n", genome->network->current->id);

    nextNeuron(genome->network);
  }

  fprintf(f, "}");
  fclose(f);

  return 1;
}
