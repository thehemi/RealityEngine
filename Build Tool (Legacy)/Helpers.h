#ifndef HELPERS_H
#define HELPERS_H


Vector AsD3DPoint(Vector& p);
void ConvertMaxMatrix(Matrix& MaxMatrix);
int CountFaces(Node& node);

void Sort(Vector& bmin, Vector& bmax);
int Inside(BBox& b, Vector& p);
TouchType Touches(BBox& b1, BBox& b2);


#endif