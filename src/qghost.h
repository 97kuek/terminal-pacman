#ifndef TERMINAL_PACMAN_QGHOST_H
#define TERMINAL_PACMAN_QGHOST_H

#define QGHOST_STATES 256
#define QGHOST_ACTIONS 4

typedef struct QGhost {
    float q[QGHOST_STATES][QGHOST_ACTIONS];
    unsigned int rng;   /* internal PRNG state */
    long steps;         /* number of selections made (drives epsilon decay) */
} QGhost;

/* Zero the table, seed the PRNG, reset steps. */
void qghost_init(QGhost *q, unsigned int seed);

/* Epsilon-greedy action selection restricted to actions where valid[a] != 0.
   Epsilon decays from ~0.5 toward ~0.05 as `steps` grows (e.g. eps = 0.05 +
   0.45 / (1 + steps/400.0)). With probability epsilon pick a uniform random
   VALID action; otherwise pick the valid action with the highest Q value
   (break ties by lowest index). Increments steps. Returns the chosen action
   index 0..3, or -1 if no action is valid. */
int qghost_select(QGhost *q, int state, const unsigned char valid[QGHOST_ACTIONS]);

/* Q-learning update:
   Q[s][a] += ALPHA * (reward + GAMMA * max_{a' : valid2[a']} Q[s2][a'] - Q[s][a])
   with ALPHA = 0.2f, GAMMA = 0.9f. If no next action is valid, treat the future
   term as 0. No-op if state/action/next_state are out of range. */
void qghost_update(QGhost *q, int state, int action, float reward,
                   int next_state, const unsigned char valid2[QGHOST_ACTIONS]);

#endif
