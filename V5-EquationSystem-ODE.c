#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h> //we like to know how long things take. 
//having not used time.h before we refered to
//https://en.wikibooks.org/wiki/C_Programming/time.h

//Note: math.h requries the "-lm" arg be added at the END of tasks.json's arguments.
//https://askubuntu.com/questions/332884/how-to-compile-a-c-program-that-uses-math-h

//ODE Solver
//By G. M. Steward
//The main goal of this project is to solve Ordinary Differential Equation Systems
//in complete generality. However, we will take a while to get there.
//This fifth version seeks to allow the program to solve a system of ODEs, starting with a very simple one.
//User functionality will (eventually) involve input of own equations, but we don't have that here yet, and it may be a while.

//Heavily influenced by Numerical Mathematics and Computing 6E by Cheney and Kincaid.

//Outside the program, we substantiate the differential equation itself.

double diffyQEval (double x, double y[], int i)
{
    switch(i){
        case 0: {
            return y[1]; 
            //we use 0 to represent the answer we eventually want to get
            //in this case, the solution to y'' = y+x, stated here as y'=z
            break;
        }
        case 1: {
            return y[0] + x;
            //1 represents the first derivative. y''=y+x, but when we split it up we got y'=z. This equation returns the value
            //for the z derivative, z'=y+x
            break;
        }
        // case...
        //add more cases for higher order systems. 
        default: {
            //Any number could be a result, theoretically, so there isn't an easy error value to put here.
        }
    }
    //This is the differential equation system itself. 
    //By default we have a very simple y'' = y+x situation here, split up into
    // y[0]' = y[1]
    // y[1]' = y[0]+x
    //Naturally other equaitons can be put in, but be sure to change the numberOfEquations value!
    //feel free to change the return values to other functions. 
    //Note: not guaranteed to work for functions that are not well-behaved. 
    //We have this set up so y[0] would be the final "answer" for a split-up higher-order ODE. 
}

//This is the function to evaluate the known solution. Must be set manually.
double knownQEval (double x)
{
    return exp(x) + exp(-x) - x;
    //the known solution to the differential equaiton, specifically what we call y[0]
    //used to measure relative errors. 
    //"exp(x) + exp(-x) - x" is "default." as it is the answer to the differential equation we chose. 
    //Do note that this would change with different boundary conditions. 
}

//Remember when adjusting these to adjust the boundary value bValue in main() as well. 

int main()
{
    printf("Beginning ODE Solver \"Odie\" V4...\n");
    
    //SECTION I: Preliminaries
    //Before the program actually starts, variables need to be created
    //and set, as well as the function itself chosen. 
    //The diffyQ itself can be found declared in diffyQEval().

    //Butcher Table: for now we define our method table here. 
    //When run through the notebook this section is absent as it fills it itself. 
    //Uncomment the method you wish to use. 
    //double butcher[4][4] = {{0.0,0.0,0.0,0.0},{1.0,1.0,0.0,0.0},{0.5,0.25,0.25,0.0},{3,1.0/6.0,1.0/6.0,2.0/3.0}};
    //This is the SSPRK3 method, chosen since it has a simple array but with less zeroes than other options. 
    //Note that the "3" in the last row is the order, that slot is always empty on a butcher table. 
    //If you wish for a different method, comment out the one above and initate one of the below:

    //double butcher[2][2] = {{0.0,0.0},{1,1.0}};
    //This is Euler's Method, good for test cases since it's easy to look at. 

    //double butcher[3][3] = {{0.0,0.0,0.0},{1.0,1.0,0.0},{2,0.5,0.5}};
    //RK2

    double butcher[5][5] = {{0.0,0.0,0.0,0.0,0.0},{0.5,0.5,0.0,0.0,0.0},{0.5,0.0,0.5,0.0,0.0},{1.0,0.0,0.0,1.0,0.0},{4,1.0/6.0,1.0/3.0,1.0/3.0,1.0/6.0}};
    //RK4

    /*double butcher[7][7] = {{0.0,0.0,0.0,0.0,0.0,0.0,0.0},
    {0.2,0.2,0.0,0.0,0.0,0.0,0.0},
    {0.3,3.0/40.0,9.0/40.0,0.0,0.0,0.0,0.0},
    {0.6,0.3,-9.0/10.0,1.2,0.0,0.0,0.0},
    {1.0,-11.0/54.0,2.5,-70.0/27.0,35.0/27.0,0.0,0.0},
    {7.0/8.0,1631.0/55296.0,175.0/512.0,575.0/13824.0,44275.0/110592.0,253.0/4096.0,0.0},
    {5,37.0/378.0,0.0,250.0/621.0,125.0/594.0,0.0,512.0/1771.0}};*/
    //RK5 (Cash-Karp version)

    //How to get array size: https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
    size_t methodSize = sizeof(butcher)/sizeof(butcher[0][0]);
    int dimension = sqrt((int)methodSize);
    printf("Method Order: %i. \nOrder of Error should be near Method Order + 1.\n",(int)butcher[dimension-1][0]);
    printf("If not, try a larger step size, roundoff error may be interfering.\n");

    double step = 0.01; //the "step" value.
    double bound = 0.0; //where the boundary/initial condition is. Same for every equation in the system. 
    int numberOfEquations = 2; //How many equations are in our system?
    double bValue[numberOfEquations]; 
    bValue[0] = 2.0;
    bValue[1] = -1.0;
    //the value at y(bound). By default we say y[0](0) = 0 and y[1](0) = 1
    //There has to be a better way to do this, may involve working in the exterior python, we shall see. 
    const int SIZE = 1000; //How many steps we are going to take?
    bool validate = true; //set to true if you wish to run a validation test.
    //Attempts to find the order of the method used. 

    double y1[numberOfEquations];
    double y2[numberOfEquations];
    double yTruth1;
    double yTruth2;
    double yError;
    //These variables temporarily store the values calculated before they are 
    //printed to the output file and forgotten. 
    //y is what we solve for, yTruth is the "known results."
    //yError is the error of y when compared to yTruth
    //Should be able to handle any size, unlike arrays, which just hog memory. 
    //Errors and Truth are not arrays as they are only concerned with the final value. 

    y1[0] = bValue[0]; //boundary condition, have to start somewhere.
    y1[1] = bValue[1]; //boundary condition, have to start somewhere.
    yTruth1 = knownQEval(bound);
    yError = 0; //has to be zero as they must match at this point. 
    //If not, the boundary conditions were not stated properly. 

    double saveErr1=0, saveErr2=0; //variables for validation if requested. 
    
    //SECTION II: The Loop

    //prior to beginning the loop, start the timer. 
    double startN = time(NULL);
    //printf("Time: %.90f seconds\n", startN);
    //set start time to current time.  Uncomment to print.

    //also open the file we'll be writing data to. 
    FILE *fp;
    fp = fopen("oData.txt","w");

    //This loop fills out all the data.
    //It takes a provided butcher table and executes the method stored within. Any table should work. 
    printf("INITIAL: Position:\t%f\tTruth:\t%10.9e\tCalculated:\t%10.9e\tError:\t%10.9e\t\n",bound,yTruth1, y1[0], yError);
    fprintf(fp,"Position:,\t%f,\tTruth:,\t%10.9e,\tCalculated:,\t%10.9e,\tError:,\t%10.9e,\t\n",bound,yTruth1, y1[0], yError);
    //Comma delimiters are printed to the file so it can be converted to .csv with ease. 
    
    for (int i = 0; i < SIZE; i++){
        //for (int m = 0; m < numberOfEquations; m++) {    
            //printf("%i",m);
            double k[dimension]; //This is obsolete, it's here to keep validation from complaining until its revamped.

            double kA[dimension]; //Create an array that can store all the steps needed for the method.
            double kB[dimension]; //Create an array that can store all the steps needed for the method.
            //each diffyQ has its own set of K-values, both are needed. 
            //Naturally a 2D array will replace these once we need to get more general. 

            double yInsert[numberOfEquations];
            //we do have an array for the inserted y-values for each equation though. 

            yInsert[0] = y1[0]; //The form of the y-insertion into our function changes, so it needs a loop.
            yInsert[1] = y1[1]; //Manually setting for now. 

            for (int j = 1; j < dimension; j++) {
                //Due to the way the Butcher Table is formatted, start our index at 1 and stop at the end. 
                double xInsert = bound+i*step + butcher[j-1][0]*step;
                //x does not change much for different tables, just adjust the "step correction" term.

                for (int n = 1; n < dimension; n++) {
                    //Once again, start at index of 1 rather than 0. 
                    yInsert[0] = yInsert[0] + butcher[j-1][n]*kA[n];
                    yInsert[1] = yInsert[1] + butcher[j-1][n]*kB[n];
                    //Each individual y portion is dependent on one or the other of the k values. 
                    //Note that even though this is called on the first pass through, butcher tables are triangular
                    //And thus always have zeroes which do not care what value is in k.
                    //This is probably bad programming practice though and an explicit checker should be added.
                }
                kA[j] = step*diffyQEval(xInsert,yInsert,0); //calculate the complete k-value. 
                kB[j] = step*diffyQEval(xInsert,yInsert,1); //calculate the complete k-value, round 2. 
                if (i == 0) {
                    printf("%i, %10.9e\n",j, kA[j]);
                    printf("%i, %10.9e\n",j, kB[j]);
                    //Print out what the first k values are. 
                }
            }
            //Now that we have all the k-values set, we need to find the actual result in one final loop.
            //The sum for the first set... 
            kA[0] = y1[0];
            for (int j = 1; j < dimension; j++) {
                kA[0] = kA[0] + butcher[dimension-1][j]*kA[j];
            }
            y2[0] = kA[0];
            //The sum for the second set. 
            kB[0] = y1[1];
            for (int j = 1; j < dimension; j++) {
                kB[0] = kB[0] + butcher[dimension-1][j]*kB[j];
            }
            y2[1] = kB[0];
            
            yTruth2 = knownQEval(bound+step*(i+1));
            yError = (yTruth2 - y2[0]);

            //After each step is calculated, print results. 
            //printf("Position:\t%f\tTruth:\t%10.9e\tCalculated:\t%10.9e\tError:\t%10.9e\t\n",bound+(i+1)*step,yTruth2, y2, yError);
            //uncomment if you want live updates. 
            //if (m==0) {
                fprintf(fp,"Position:,\t%f,\tTruth:,\t%10.9e,\tCalculated:,\t%10.9e,\tError:,\t%10.9e,\t\n",bound+(i+1)*step,yTruth2, y2[0], yError);
            //}
                    
            //validation: grab the first nonzero error, calculate its order.
            //Currently broken. 
            if(validate==false && i == 0.0) { //currently set to ignore this. REMEMBER TO TURN BACK ON LATER!
                //Only activate on first step. 
                saveErr1 = yError;

                //The following is an algorithm for determining the rate of error 
                //convergence. A bit rudimentary, could be condensed, but is also only
                //called once so not relaly a concern and it is easier to read this way. 
                for (int j = 1; j < dimension; j++) {
                    double xInsert = bound+i*step*0.5 + butcher[j-1][0]*step*0.5;
                    double yInsert[numberOfEquations];
                    yInsert[0] = y1[0];
                    yInsert[1] = y1[1];
                    for (int n = 1; n < dimension; n++) {
                        yInsert[0] = yInsert[0] + butcher[j-1][n]*k[n];
                        yInsert[1] = yInsert[1] + butcher[j-1][n]*k[n];
                    }
                    k[j] = step*0.5*diffyQEval(xInsert,yInsert,0);
                }
                k[0] = y1[0];
                for (int j = 1; j < dimension; j++) {
                    k[0] = k[0] + butcher[dimension-1][j]*k[j];
                }
                double truthValidate = knownQEval(bound+step*0.5);
                saveErr2 = (truthValidate - k[0]);
                //Basically we just calculated the initial error for half step size. 
                //Now we can compare using the equation for order estimation:
                double order =  log2(saveErr1/saveErr2);
                printf("Order of Error: %f\n", order);
            }
            y1[0]=y2[0];
            y1[1]=y2[1];
            //make sure to assign both variables to the next step. 
            yTruth1=yTruth2;
        //}
         
    }

    //SECTION III: Analysis
    //Post-processing goes here.

    // basic reference: https://www.tutorialspoint.com/cprogramming/c_file_io.htm
    // used to be a file converter here, now there isn't, we just close the file. 
    fclose(fp);

    //TIMER
    double endN = time(NULL);
    //loop is complete, how long did it take?
    printf("Time Elapsed: %f seconds\n", endN-startN);
    //Only calculates to the closest second, for some reason.

    //BONUS BOI: we need to check our sanity.
    double kkA[5];
    double kkB[5];
    double yBuffer[2] = {2,-1};
    kkA[1] = step*diffyQEval(0, yBuffer ,0);
    printf("%i, %10.9e\n",1, kkA[1]);
    kkB[1] = step*diffyQEval(0, yBuffer ,1);
    printf("%i, %10.9e\n",1, kkB[1]);

    yBuffer[0] = 2.0 + step*kkA[1]*0.5;
    yBuffer[1] = -1.0 + step*kkB[1]*0.5;
    kkA[2] = step*diffyQEval(step*0.5, yBuffer ,0);
    printf("%i, %10.9e\n",2, kkA[2]);
    kkB[2] = step*diffyQEval(step*0.5, yBuffer ,1);
    printf("%i, %10.9e\n",2, kkB[2]);

    yBuffer[0] = 2.0 + step*kkA[2]*0.5;
    yBuffer[1] = -1.0 + step*kkB[2]*0.5;
    kkA[3] = step*diffyQEval(step*0.5, yBuffer ,0);
    printf("%i, %10.9e\n",3, kkA[3]);
    kkB[3] = step*diffyQEval(step*0.5, yBuffer ,1);
    printf("%i, %10.9e\n",3, kkB[3]);

    printf("ODE Solver \"Odie\" V4 Shutting Down...\n");
    return 0;
}

// - GM, master of dogs.