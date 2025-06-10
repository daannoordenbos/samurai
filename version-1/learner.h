#ifndef LEARNER_H_INCLUDED
#define LEARNER_H_INCLUDED

double evaluation(const Position& pos);
void partialEval(const Position& pos, double score);

void descent();

void evaluationStep(double samples);

void saveParameters();

void loadParameters();
void    randomWeights   ();
void    counts   ();
#endif // LEARNER_2_H_INCLUDED
