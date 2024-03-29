#include "nrpy_odiegm_funcs.c" // nrpy_odiegm itself.
#include "nrpy_odiegm_user_methods.c" // User-dependent functions. 

#include <string.h>
// This #include is not usually part of Odie, it's here to process ETK inputs. 

// This file is technically not part of Odie, it is just an example implementation.
// However, it is exceptionally versatile, and can be used to run virtually 
// every system of differential equations desired.
// That said, since it's general, it trades off some efficiency to handle
// every type of method. If high efficiency is desired, it is recommended that the user 
// make a custom main(). 


double EOS_table_values_nabber(double T_initial, double **logrho, double **logpres, double **logeps, int *array_size);
// fancy C++ prototype for interfacing with EOS_Omni

void nrpy_odiegm_main(CCTK_ARGUMENTS)
{
    // declare values needed by ETK. 
    DECLARE_CCTK_PARAMETERS
    printf("Beginning ODE Solver \"Odie\" V10...\n");
    
    // Before anything, get the tables read. 
    // They are declared in the user_method.c file so they can be used there. 
    
    if (TOVOdieGM_use_EOS_table == true) {
    	EOS_table_values_nabber(TOVOdieGM_T_initial, &logrho, &logpres, &logeps, &array_size);
    	// Get the values we need from the EOS table. 
    }
    // If we aren't using a table we simply don't do this. 



    // SECTION I: Preliminaries

    // Before the program actually starts, variables need to be created
    // and set, as well as the functions chosen. 
    // The system of differential equations can be found declared in diffy_Q_eval
    // in nrpy_odiegm_user_methods.c

    double step = TOVOdieGM_step; // the "step" value. Initial step if using an adaptive method.
    double current_position = 0.0; // where the boundary/initial condition is. 
    // Same for every equation in the system.
    int number_of_equations = 4; // How many equations are in our system?
    int number_of_constants = 1; // How many constants do we wish to separately evaluate and report? 
    // If altering the two "numberOf" ints, be careful it doesn't go over the actual number 
    // and cause an overflow in the functions in nrpy_odiegm_user_methods.c
    const int size = TOVOdieGM_size; // How many steps are we going to take? 
    // This is the default termination condition. 
    int adams_bashforth_order = TOVOdieGM_adams_bashforth_order; // If using the AB method, specify which order you want.
    // If we are not using the AB method this is set to 0 later automatically. 4 by default. 
    bool no_adaptive_step = TOVOdieGM_no_adaptive_step; // Sometimes we just want to step forward uniformly 
    // without using GSL's awkward setup. False by default. 

    // bool report_error_actual = TOVOdieGM_report_error_actual; // not possible in this implementation. 
    bool report_error_estimates = TOVOdieGM_report_error_estimates;
    // AB methods do not report error estimates. 
    // BE WARNED: setting reporError (either kind) adjusts the output file, adding four values to the file after each line. 
    // Make sure the file reading program handles it properly, whatever ends up being used. 

    // ERROR PARAMETERS: Use these to set limits on the erorr. 
    double absolute_error_limit = TOVOdieGM_absolute_error_limit; // How big do we let the absolute error be?
    double relative_error_limit = TOVOdieGM_relative_error_limit; // How big do we let the relative error be?
    // Default: 1e-14 for both.
    // Note: there are a lot more error control numbers that can be set inside the 
    // control "object" (struct) d->c.
    
    //Since C doesn't have strings we have to work around this. 
    //We do this by opening the file early, but doing it directly. 
    FILE *fp2;
    fp2 = fopen(TOVOdieGM_file_name,"w");
    printf("Printing to file '%s'.\n",TOVOdieGM_file_name);

    // Open the file we'll be writing data to. 

    // Now we set up the method. 
    
    const nrpy_odiegm_step_type * step_type;
    // Here is where the method is actually set, by specific name since that's what GSL does.     
    const nrpy_odiegm_step_type * step_type_2;
    // This is a second step type "object" (struct) for hybridizing. 
    // Only used if the original type is AB.
    // Set to AB to use pure AB method. 
    
    // Since we take input directly from ETK for this and C (as far as we know) doesn't let us use strings to 
    // define names, we are going to put an ugly switch statement here. 
    // turns out you can't actually switch strings in C. Great. ChatGPT was used to adjust this. 
    
	if (strcmp("Euler",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_euler;
	} else if (strcmp("RK2H",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK2_Heun;
	} else if (strcmp("RK2M",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK2_MP;
	} else if (strcmp("RK2R",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK2_Ralston;
	} else if (strcmp("RK3",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK3;
	} else if (strcmp("RK3H",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK3_Heun;
	} else if (strcmp("RK3R",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK3_Ralston;
	} else if (strcmp("RK3S",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_SSPRK3;
	} else if (strcmp("RK4",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_RK4;
	} else if (strcmp("DP5",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_DP5;
	} else if (strcmp("CK5",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_DP5alt;
	} else if (strcmp("CK5A",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_CK5;
	} else if (strcmp("DP6",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_DP6;
	} else if (strcmp("L6",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_L6;
	} else if (strcmp("DP8",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_DP8;
	} else if (strcmp("AHE",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_AHE;
	} else if (strcmp("ABS",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_ABS;
	} else if (strcmp("ARKF",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_ARKF;
	} else if (strcmp("ACK",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_ACK;
	} else if (strcmp("ADP5",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_ADP5;
	} else if (strcmp("ADP8",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_ADP8;
	} else if (strcmp("AB",TOVOdieGM_step_type) == 0) {
	    step_type = nrpy_odiegm_step_AB;
		if (strcmp("Euler",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_euler;
		} else if (strcmp("RK2H",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK2_Heun;
		} else if (strcmp("RK2M",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK2_MP;
		} else if (strcmp("RK2R",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK2_Ralston;
		} else if (strcmp("RK3",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK3;
		} else if (strcmp("RK3H",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK3_Heun;
		} else if (strcmp("RK3R",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK3_Ralston;
		} else if (strcmp("RK3S",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_SSPRK3;
		} else if (strcmp("RK4",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_RK4;
		} else if (strcmp("DP5",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_DP5;
		} else if (strcmp("CK5",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_DP5alt;
		} else if (strcmp("CK5A",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_CK5;
		} else if (strcmp("DP6",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_DP6;
		} else if (strcmp("L6",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_L6;
		} else if (strcmp("DP8",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_DP8;
		} else if (strcmp("AHE",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_AHE;
		} else if (strcmp("ABS",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_ABS;
		} else if (strcmp("ARKF",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_ARKF;
		} else if (strcmp("ACK",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_ACK;
		} else if (strcmp("ADP5",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_ADP5;
		} else if (strcmp("ADP8",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_ADP8;
		} else if (strcmp("AB",TOVOdieGM_step_type_2) == 0) {
		    step_type_2 = nrpy_odiegm_step_AB;
		}
	}
	
    // We usually don't declare these, but we want ETK to be able to edit them.
    double scale_factor = TOVOdieGM_scale_factor; // A scale factor used in the error comparison formula.
    double error_safety = TOVOdieGM_error_safety; // A factor that limits how drastically things can change for stability.
    double ay_error_scaler = TOVOdieGM_ay_error_scaler; // Weight given to error estimates related to the function itself.
    double ady_error_scaler = TOVOdieGM_ady_error_scaler; // Weight given to error estimates related to the function's derivative.
    double max_step_adjustment = TOVOdieGM_max_step_adjustment; // What is the largest growing step adjustment we'll allow?
    double min_step_adjustment = TOVOdieGM_min_step_adjustment; // What is the smallest shrinking step adjustment we'll allow?
    double absolute_max_step = TOVOdieGM_absolute_max_step; // Largest allowed step?
    double absolute_min_step = TOVOdieGM_absolute_min_step; // Smallest allowed step?
    double error_upper_tolerance = TOVOdieGM_error_upper_tolerance; // If estimated error is higher than this, it is too high. 
    double error_lower_tolerance = TOVOdieGM_error_lower_tolerance; // If estimated error is lower than this, it is too low.

    // AFTER THIS POINT THERE SHOULD BE NO NEED FOR USER INPUT, THE CODE SHOULD HANDLE ITSELF. 

    // We need to define a struct that can hold all possible constants. 
    struct constant_parameters cp; 
    cp.dimension = number_of_constants;
    // We'll set the actual parameters later. 
    // Do note that cp itself needs to be declared in constant_parameters in 
    // nrpy_odiegm_user_methods.c manually.
    // The methods that make use of it it need to be declared as well, if they are used.

    nrpy_odiegm_system system = {diffy_Q_eval,known_Q_eval,number_of_equations,&cp};
    // This is the system of equations we solve.
    // The second slot was originally the Jacobian in GSL, but we use it to pass a 
    // true answer function that may or may not be used.

    nrpy_odiegm_driver *d;
    d = nrpy_odiegm_driver_alloc_y_new(&system, step_type, step, absolute_error_limit, relative_error_limit); 
    // This is the "object" (struct) that runs everything, contains every needed varaible, etc. 
    // Basically the master of the whole thing, hence why it's called the "driver"
    // Contains three major sub-objects besides the step type. 
    // c is the controller, which is primarily used to store adaptive timestep values. 
    // s is the step, which has the step type in it, but also parameters that describe the steps.
    // e is the evolver, which actually performs the update when it is requested. 
    
    //ETK interface: make sure all possible adjusted error parameters are taken care of. 
    d->c->scale_factor = scale_factor;
    d->c->error_safety = error_safety;
    d->c->ay_error_scaler = ay_error_scaler;
    d->c->ady_error_scaler = ady_error_scaler;
    d->c->max_step_adjustment = max_step_adjustment;
    d->c->min_step_adjustment = min_step_adjustment;
    d->c->absolute_max_step = absolute_max_step;
    d->c->absolute_min_step = absolute_min_step;
    d->c->error_upper_tolerance = error_upper_tolerance;
    d->c->error_lower_tolerance = error_lower_tolerance;

    int method_type = 1;
    if (step_type->rows == step_type->columns) {
        method_type = 0; // AKA, normal RK-type method. 
    } // No need for an else, we set it to 1 earlier to represent Adaptive methods. 
    if (step_type->rows == 19) { 
        method_type = 2;
    } else {
        adams_bashforth_order = 0;
    }
    d->s->adams_bashforth_order = adams_bashforth_order;
    d->e->no_adaptive_step = no_adaptive_step;
    // Based on what type of method we are using, we adjust some parameters within the driver.

    if (method_type == 2) {
        printf("Method Order: %i.\n",adams_bashforth_order);
    } else {
        printf("Method Order: %i.\n",step_type->order);            
    }
    
    double y[number_of_equations];
    // These next few variables temporarily store the values calculated before they are 
    // printed to the output file and forgotten.
    // y contains the values of the actual equations. 
    // Each array only holds values at one evaluation point, but one for each Equation.

    double c[number_of_constants];
    // c is just used to hold any constants we wish to report. 
    // You'd think that, since we have the constants in a struct, we can avoid declaring this.
    // No. Not as far as we can tell, anyway. Structs are a pain to iterate through,
    // and we can't know what form the user is going to hand us the struct in. 

    // This here sets the initial conditions as declared in get_initial_condition
    get_initial_condition(y); 
    const_eval(current_position, y,&cp);
    assign_constants(c,&cp);

    // Before continuing, let's print out our initial data. 
    // The print function is automatically adaptable to any size of data. 
    // We print both to the terminal and to the file for the initial conditions, 
    // but later only print to the file.
    printf("Data given as radius, energy density, baryon density, pressure, mass, 2ln(lapse), polytropic radius, {estimated errors: pressure, 2ln(lapse), mass, polytropic radius}\n");
    printf("INITIAL: %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e ", current_position, c[0], TOVOdieGM_central_baryon_density, 
    			y[0], y[2], y[1], y[3]);
    fprintf(fp2,"%15.14e %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e ", current_position, c[0], TOVOdieGM_central_baryon_density, 
    			y[0], y[2], y[1], y[3]);
    if (report_error_estimates == true) {
        printf("%15.14e %15.14e %15.14e %15.14e ",0.0,0.0,0.0,0.0);
        fprintf(fp2, "%15.14e %15.14e %15.14e %15.14e ",0.0,0.0,0.0,0.0); 
    }
    printf("\n");
    fprintf(fp2,"\n");
    
    // What follows is the original printing routine, we replaced this for ETK interface purposes. 
    /* // First, print the location we are at. 
    printf("INITIAL: Position:,\t%f,\t",current_position);
    fprintf(fp2, "Position:,\t%15.14e,\t",current_position);
    // Second, go through and print the result for every single equation in our system.
    for (int n = 0; n < number_of_equations; n++) {
        printf("Equation %i:,\t%15.14e,\t",n, y[n]);
        fprintf(fp2, "Equation %i:,\t%15.14e,\t",n, y[n]);
    }
    // Third, print out desired constants.
    assign_constants(c,&cp);    
    for (int n = 0; n < number_of_constants; n++) {
        printf("Constant %i:,\t%15.14e,\t",n, c[n]);
        fprintf(fp2, "Constant %i:,\t%15.14e,\t",n, c[n]);
    }
    // Lastly, the newline character. 
    printf("\n");
    fprintf(fp2,"\n");
    // Comma delimiters are printed to the file so it can be read as .csv with ease. 

    if (report_error_estimates == true) {
        // In order to keep things neat and regular in the file, print a first line of errors. 
        // Even though by necessity all of them must be zero. 
        fprintf(fp2, "Errors Estimates:,\t");
        for (int n = 0; n < number_of_equations; n++) {
            fprintf(fp2, "Equation %i:,\t0.0,\t",n);
        }
        for (int n = 0; n < number_of_constants; n++) {
            fprintf(fp2, "Constant %i:,\t0.0,\t",n);
        }   
        fprintf(fp2,"\n");
    } */

    // SECTION II: The Loop

    // This loop fills out all the data.
    // It takes a provided butcher table and executes the method stored within. 
    // Any RK table should work, even one not included by default.
    // Also handles AB methods up to 19th order. No one should ever need more. 

    for (int i = 0; i < size; i++){
        
        // Hybrid Methods require some fancy footwork, hence the if statements below. 
        if (method_type == 2 && i == 0 && step_type_2 != nrpy_odiegm_step_AB) {
            d->s->type = step_type_2;
            d->s->rows = step_type_2->rows;
            d->s->columns = step_type_2->columns;
            d->s->method_type = 0;
            d->s->adams_bashforth_order = adams_bashforth_order;
            d->e->no_adaptive_step = true;
        } else if (step_type != step_type_2 && method_type == 2 && i == adams_bashforth_order) {
            d->s->type = step_type;
            d->s->rows = step_type->rows;
            d->s->columns = step_type->columns;
            d->s->method_type = 2;
            d->s->adams_bashforth_order = adams_bashforth_order;
            d->e->no_adaptive_step = true;
        }

        nrpy_odiegm_evolve_apply(d->e, d->c, d->s, &system, &current_position, current_position+step, &step, y);
        // This is the line that actually performs the step.

        exception_handler(current_position,y);
        const_eval(current_position,y,&cp);
        assign_constants(c,&cp);
        // These lines are to make sure the constant updates. 
        // And exception constraints are applied.  

        // Printing section. Altered for ETK use. 
	    /* printf("%15.14e %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e ", current_position, c[0], pow(y[0]/TOVOdieGM_K,1.0/TOVOdieGM_Gamma), 
	    			y[0], y[2], y[1], y[3]);  */
	    fprintf(fp2,"%15.14e %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e ", current_position, c[0], pow(y[0]/TOVOdieGM_K,1.0/TOVOdieGM_Gamma), 
	    			y[0], y[2], y[1], y[3]);
	    if (report_error_estimates == true) {
	    	// printf("%15.14e %15.14e %15.14e %15.14e ",(d->e->yerr[0]),(d->e->yerr[1]),(d->e->yerr[2]),(d->e->yerr[3])); 
                fprintf(fp2, "%15.14e %15.14e %15.14e %15.14e ",(d->e->yerr[0]),(d->e->yerr[1]),(d->e->yerr[2]),(d->e->yerr[3])); 
	    }
	    // printf("\n");
	    fprintf(fp2,"\n");
		
        
        
        /* // Uncomment for live updates. Prints to the file automatically.
        // printf("Position:,\t%15.14e,\t",current_position);
        fprintf(fp2, "Position:,\t%15.14e,\t",current_position);
        for (int n = 0; n < number_of_equations; n++) {
            // printf("Equation %i:,\t%15.14e,\t",n, y[n]);
            fprintf(fp2, "Equation %i:,\t%15.14e,\t",n, y[n]);
        }

        for (int n = 0; n < number_of_constants; n++) {
            // printf("Constant %i:,\t%15.14e,\t",n, c[n]);
            fprintf(fp2, "Constant %i:,\t%15.14e,\t",n, c[n]);
            // printf("Constant %i:,\t%15.14e %15.14e,\n",n, c[n], y[n]);
        }
        // printf("\n");
        fprintf(fp2,"\n");

        if (report_error_estimates == true) {
            // Print the error estimates we already have. 
            fprintf(fp2, "Error Estimates:,\t");
            for (int n = 0; n < number_of_equations; n++) {
                fprintf(fp2, "Equation %i:,\t%15.14e,\t",n,(d->e->yerr[n])); 
            }
            // Constant estimates not reported, only differential equation values. 
            fprintf(fp2,"\n");
        } */

        if (do_we_terminate(current_position, y, &cp) == 1) {
            i = size-1;
            // If we need to bail, set i to size-1 to break the loop. The -1 is there to make sure final line printing works. 
        } 
        if (i == size-1) {
       	    // Also potentially a good idea: print the final line. 
            
            printf("FINAL: %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e %15.14e ", current_position, c[0], pow(y[0]/TOVOdieGM_K,1.0/TOVOdieGM_Gamma), 
	    			y[0], y[2], y[1], y[3]);
	    if (report_error_estimates == true) {
	    	printf("%15.14e %15.14e %15.14e %15.14e ",(d->e->yerr[0]),(d->e->yerr[1]),(d->e->yerr[2]),(d->e->yerr[3]));  
	    }
	    printf("\n");
            
            /* printf("FINAL: Position:,\t%15.14e,\t",current_position);
            for (int n = 0; n < number_of_equations; n++) {
                // printf("Equation %i:,\t%15.14e,\t",n, y[n]);
                printf("Equation %i:,\t%15.14e,\t",n, y[n]);
            }

            for (int n = 0; n < number_of_constants; n++) {
                // printf("Constant %i:,\t%15.14e,\t",n, c[n]);
                printf("Constant %i:,\t%15.14e,\t",n, c[n]);
                // printf("Constant %i:,\t%15.14e %15.14e,\n",n, c[n], y[n]);
            }
            // printf("\n");
            printf("\n"); */
        }
    }

    // SECTION III: Analysis

    // Minor post-processing goes here. 
    // Anything advanced will need to be done in a data analysis program. 
    // We like to use matplotlib for python.

    fclose(fp2);

    nrpy_odiegm_driver_free(d);
    // MEMORY SHENANIGANS
    // We need to free these arrays that were declared in the C++ file.
    free(logrho);
    free(logpres);
    free(logeps);
    // Chat GPT confirms that this ladder of poitner nonsense
    // is actually freeing the memory, nothing's left hanging. 

    printf("ODE Solver \"Odie\" V10 Shutting Down...\n");

}

// - GM, master of dogs.
