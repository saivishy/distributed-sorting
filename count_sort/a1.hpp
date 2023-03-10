/*  SAI_VISHWANATH
 *  VENKATESH
 *  saivishw
 */

#ifndef A1_HPP
#define A1_HPP

#include <vector>


void isort(std::vector<short int>& Xi, MPI_Comm comm) {

    int size, rank, root=0;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);    
    
//count locally 
    short int range_min = -size+1, range_max = size-1, range = ((2*size) - 1); 
    std::vector<long long int> freq(range,0);
    for(long long int i=0;i<Xi.size();i++){
        freq[Xi[i]-range_min]++;
    }

//to pad the 1st processor with 0
    freq.push_back(0);
    std::rotate(freq.rbegin(), freq.rbegin() + 1, freq.rend()); 

//implement minimal Alltoall to share local tables
    std::vector<long long int> recvBuf((size*2),0);

    MPI_Alltoall(&freq[0], 2, MPI_LONG_LONG, &recvBuf[0], 2, MPI_LONG_LONG, comm); 

//get global count
    long long int sum1=0, sum2=0, num1, num2;
    for(long long int i=0;i<recvBuf.size();i++){
        if (i%2==0){
            sum1+=recvBuf[i];
        }

        else{
            sum2+=recvBuf[i];
        }
    }

// generate Xi on each rank using sum1 and sum2
    if (rank!=root){
        num2 = range_min+(2*rank);
        num1 = num2-1;
        Xi.resize(sum1+sum2,num1);
        for (int i=0;i<Xi.size();i++){
            if (i<sum1){
                Xi[i]=num1;
            }
            else{
                Xi[i]=num2;
            }
        }
    }

//same for root 
    if (rank==root){
        Xi.resize(sum2,range_min);
        for (int i=0;i<Xi.size();i++){
            Xi[i]=range_min;
        }
    }
} // isort

#endif // A1_HPP

