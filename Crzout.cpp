
double determinant(double** c){
    return c[0][0]*(c[1][1]*c[2][2]-c[1][2]*c[2][1])
        +c[0][1]*(c[1][2]*c[2][0]-c[1][0]*c[2][1])
        +c[0][2]*(c[1][0]*c[2][1]-c[1][1]*c[2][0]);
}

double calculate(const int dim, double* arr, const int p){
    double* arr_p= new double [dim];
    for (int i = 0; i < dim; i++)
    {
        arr_p[i]=pow(arr[i],p);
    }
    int s=0;
    for (int i = 0; i < dim; i++)
    {
        s+=arr_p[i];
    }
    delete[] arr_p;
    return s;
}

double calculate(const int dim, double* arr1, double* arr2, const int p){
    double* arr_p= new double [dim];
    for (int i = 0; i < dim; i++)
    {
        arr_p[i]=pow(arr2[i],p);
    }
    int s=0;
    for (int i = 0; i < dim; i++)
    {
        s+=arr1[i]*arr_p[i];
    }
    delete[] arr_p;
    return s;
}


//r variable
double r_c,r_o;
double r_out,delta_r,psi_c,psi_o,tangent;

//z variable
#define dim_z 6

double z_o[dim_z];
double psi[dim_z];
double z_out;

//coefficients of equation
double c1[3][3];
double c2[3];
double cA[3][3];
double cB[3][3];

//determinants
double dc,dA,dB;

double A,B;

// void setup(){
//     // //read r_c,r_o
//     // {

//     // }
//     // //read z_o
//     // {

//     // }
// }
    
// void loop(){
    // //read psi_c,psi_o,tangent for r_out
    // {

    // }
    // //read psi for z_out
    // {

    // }

    //rout
    delta_r = (psi_c-psi_o)/tangent;
    r_out = (r_c+r_o+delta_r)/2;


    //zout
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            c1[i][j]=calculate(dim_z,z_o,i+j);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        c2[i]=calculate(dim_z,psi,z_o,i);
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if(j==2){
                cA[i][j]=c2[i];
            }
            else{
                cA[i][j]=c1[i][j];
            }
            if(j==1){
                cB[i][j]=c2[i];
            }
            else{
                cA[i][j]=c1[i][j];
            }
        }
    }

    dc=determinant(c1);
    dA=determinant(cA);
    dB=determinant(cB);

    A=dA/dc;
    B=dB/dc;

    z_out=-B/(2*A);
}

