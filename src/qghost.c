#include "qghost.h"

#define QGHOST_ALPHA 0.2f
#define QGHOST_GAMMA 0.9f
#define QGHOST_SEED_FALLBACK 0x6d2b79f5u

static unsigned int qghost_next(QGhost *q)
{
    unsigned int x;

    x = q->rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    q->rng = x;

    return x;
}

static float qghost_random_unit(QGhost *q)
{
    return (float)(qghost_next(q) >> 8) * (1.0f / 16777216.0f);
}

static int qghost_state_valid(int state)
{
    return state >= 0 && state < QGHOST_STATES;
}

static int qghost_action_valid(int action)
{
    return action >= 0 && action < QGHOST_ACTIONS;
}

void qghost_init(QGhost *q, unsigned int seed)
{
    int state;
    int action;

    if (q == 0) {
        return;
    }

    for (state = 0; state < QGHOST_STATES; ++state) {
        for (action = 0; action < QGHOST_ACTIONS; ++action) {
            q->q[state][action] = 0.0f;
        }
    }

    q->rng = seed != 0u ? seed : QGHOST_SEED_FALLBACK;
    q->steps = 0;
}

int qghost_select(QGhost *q, int state, const unsigned char valid[QGHOST_ACTIONS])
{
    int action;
    int valid_count;
    int best_action;
    float best_value;
    float epsilon;

    if (q == 0 || valid == 0 || !qghost_state_valid(state)) {
        return -1;
    }

    valid_count = 0;
    best_action = -1;
    best_value = 0.0f;

    for (action = 0; action < QGHOST_ACTIONS; ++action) {
        if (valid[action] != 0u) {
            ++valid_count;
            if (best_action < 0 || q->q[state][action] > best_value) {
                best_action = action;
                best_value = q->q[state][action];
            }
        }
    }

    if (valid_count == 0) {
        return -1;
    }

    epsilon = 0.05f + 0.45f / (1.0f + (float)q->steps / 400.0f);

    if (qghost_random_unit(q) < epsilon) {
        int pick;

        pick = (int)(qghost_next(q) % (unsigned int)valid_count);
        for (action = 0; action < QGHOST_ACTIONS; ++action) {
            if (valid[action] != 0u) {
                if (pick == 0) {
                    best_action = action;
                    break;
                }
                --pick;
            }
        }
    }

    ++q->steps;
    return best_action;
}

void qghost_update(QGhost *q, int state, int action, float reward,
                   int next_state, const unsigned char valid2[QGHOST_ACTIONS])
{
    int next_action;
    int found_next;
    float best_next;
    float old_value;
    float target;

    if (q == 0 ||
        !qghost_state_valid(state) ||
        !qghost_action_valid(action) ||
        !qghost_state_valid(next_state)) {
        return;
    }

    found_next = 0;
    best_next = 0.0f;

    if (valid2 != 0) {
        for (next_action = 0; next_action < QGHOST_ACTIONS; ++next_action) {
            if (valid2[next_action] != 0u) {
                if (!found_next || q->q[next_state][next_action] > best_next) {
                    best_next = q->q[next_state][next_action];
                    found_next = 1;
                }
            }
        }
    }

    old_value = q->q[state][action];
    target = reward;
    if (found_next) {
        target += QGHOST_GAMMA * best_next;
    }

    q->q[state][action] = old_value + QGHOST_ALPHA * (target - old_value);
}
