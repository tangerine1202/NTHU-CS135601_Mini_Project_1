#ifndef SEMANTIZER_H
#define SEMANTIZER_H

Value *semantize(BTNode *);
Value *makeValueNode(int val, int unknonw_val);
void updateNodeWeight(BTNode *);
void calculateValWithOp(Value *, Value *, char *);
void setAddrVal(int, Value *);

#endif