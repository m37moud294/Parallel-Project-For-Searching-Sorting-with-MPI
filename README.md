 these the steps for running the project in parallel
Download the project
open it by microsoft visual studio
click on Debug >choose start without debugging
cancel the window of cmd
go to cmd in your device 
change directory project directory 
write command "mpiexec -n 4 PROJECT_MPI.exe"
the project will run parallel in 4 processes you can change number of processer you need to run on it
you can choose number of function you need to run it 
give the function the directory for the input file 
the function will produce the output file in the project directory .
                    # well done #
