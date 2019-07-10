//r variable
//position of loops
const double r_c=0.12000, r_o=0.699000;
double r_out,delta_r,psi_c,psi_o,tangent;

//z variable
#define dim_z 8

double psi[dim_z];
double z_out;
double A,B;

//coefficients of fitting
const double c[3][8]={
{     -0.10157325      0.10741101      0.28746355      0.37565219      0.30099177     0.17140523    -0.064258453    -0.077091960},
{      0.85464802      0.59017087      0.29084170    -0.034480695     -0.28178700     -0.40168786     -0.50693151     -0.51077351},
{       3.8203913      0.60522721      -2.2069399      -3.6896237      -2.7423851     -0.91601104       2.4718668       2.6574733}};

void setup(){
    
}
    
void loop(){

    //read psi for z_out
    //psi[0] and psi[7] are fluc loop
    //psi[1] to psi[6] are saddle loop
    for (int i = 0; i < dim_z; i++)
    {
        psi[i]=analogRead(A1+i);
    }
    for (int i = 1; i < dim_z-1; i++)
    {
        psi[i]+=psi[i-1];
    }
    
    //read psi_c,psi_o,tangent for r_out
    {
        psi_c=analogRead(A9);
        psi_o=psi[3]; //3 is outboard center
        tangent=analogRead(A10);
    }
    
    //rout
    delta_r = (psi_c-psi_o)/tangent;
    r_out = (r_c+r_o+delta_r)/2;

    //zout
    A=0,B=0;
    for (int i = 0; i < dim_z; i++)
    {
        A+=psi[i]*c[2][i];
        B+=psi[i]*c[1][i];
    }
    
    z_out=-B/(2*A);
}

