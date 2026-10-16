#include "LEDLayouts.h"
#include <assert.h>
SerialStub Serial;
Adafruit_NeoPixel pixels(416,6,0);
int main(){
 assert(LeftRing::index(0)==0);
 assert(RightRing::index(15)==415);
 assert(FaceMatrix::indexXY(0,0)==16);
 assert(FaceMatrix::indexXY(47,7)==399);
 assert(FaceMatrix::indexXY(0,1)==64);
 assert(BothRings::index(15)==15);
 assert(BothRings::index(16)==400);
 assert(BothRings::index(31)==415);
 typedef Matrix<16,48,8,MatrixOrder::Rows,true> Serp;
 assert(Serp::indexXY(0,1)==111);
 assert(Serp::indexXY(47,1)==64);
 typedef Matrix<16,48,8,MatrixOrder::Columns,true> Cols;
 assert(Cols::indexXY(1,0)==31);
 assert(Cols::indexXY(1,7)==24);
 typedef Matrix<16,48,8,MatrixOrder::Rows,false,true> Rev;
 assert(Rev::indexXY(0,0)==399);
 return 0;
}
