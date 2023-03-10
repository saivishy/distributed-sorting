#ifndef HQ_HPP
#define HQ_HPP

#include <vector>
#include <cmath>

int bin_search(std::vector<int>& Xi, int& s_val, int low, int high){
    
    // search for largest value lesser than median     
    
    if (low > high){
        return low;
    }
    else{
        int mid  = floor((low + high)/ 2);
        if (s_val==Xi[mid]){
            return mid;
        }
        else if(s_val>Xi[mid]){
            return bin_search(Xi, s_val, mid+1, high);
        }
         // (s_val>Xi[mid])
        else{
            return bin_search(Xi, s_val, low, mid-1);
        }
    } // else


} // bin_search


std::vector<int> grouping(int& d, int& size){
    std::vector<int> group;
    int val = 0;
    while(val<size){
        group.push_back(val);
        val=val+(pow(2,d));
    }
    return group;
} // grouping()

void hyperquick_sort(std::vector<int>& Xi, MPI_Comm comm) {
        int size, rank, root=0, h_dim;
        MPI_Comm_size(comm, &size);
        MPI_Comm_rank(comm, &rank);    
        MPI_Status status;
        std::vector<int> send_buf, recv_buf, group;

        
        h_dim = log(size)/log(2);
        // std::cout<<"\nRank:"<< rank <<std::endl;
        
        for (int di=h_dim;di>0; di--){

        // 1.Sort locally
            sort(Xi.begin(), Xi.end());

        // Barrier -------
            MPI_Barrier(comm);
    
        // 2. Send Group
            group=grouping(di, size);

            int median;
            // if rank in group 
            if( find(group.begin(), group.end(), rank) != group.end() ){
                // compute_median()
                median = Xi[floor(Xi.size()/2)]; // Floor division 
                // std::cout<<"\nrank "<<rank<<" is broadcasting median " << median;

                // for k (rank+1, next 2^d - 1 processors):
                for (int group_member =rank+1; group_member<rank+pow(2,di); group_member++){
                    MPI_Send(&median, 1, MPI_INT, group_member, 111, comm);
                }
                
            } // if 
            
            // Else Recv(any Processor , )
            else{
                MPI_Recv(&median, 1, MPI_INT, MPI_ANY_SOURCE, 111, comm, &status);
                // std::cout<<"\nrank "<<rank<<" recieved median "<<median<< " in  di ="<<di<<"\n";
            } //else
                

        // 3. Make small and big lists
            int div = bin_search(Xi, median, 0, Xi.size());
            // std::cout<<"\nrank "<<rank<<" binary search result "<<div<< " in  di ="<<di<<"\n";
            std::vector<int> small = std::vector<int>(Xi.begin(), Xi.begin() + div);
            std::vector<int> big = std::vector<int>(Xi.begin() + div, Xi.end() );
            
            int small_size;
            int big_size;


        // Barrier  
            MPI_Barrier(comm);    

        // 4. Swap 
            int pair = di^rank;
            // std::cout<<"\n Pairs (rank , pair) : "<< rank << ", "<<pair << " | di = "<< di << "\n";

        // 4.1 Send and Recieve sizes 
        // if rank > pair:
            if (rank>pair){

            //   Send(small, pair)
            //   Recv(big, pair)

                small_size = small.size();
                MPI_Send(&small_size, 1, MPI_INT, pair, 222, comm);
                MPI_Recv(&big_size, 1, MPI_INT, pair, 222, comm, &status);
                // std::cout<<"\nrank "<<rank<<" recieved size  "<<big_size<< " in  di ="<<di<<" from pair "<<pair<<"\n";
            }
        // else if rank < pair:
            else{
                //   Send(big, pair)
                //   Recv(small, pair)
                big_size = big.size();
                MPI_Send(&big_size, 1, MPI_INT, pair, 222, comm);
                MPI_Recv(&small_size, 1, MPI_INT, pair, 222, comm, &status);
                // std::cout<<"\nrank "<<rank<<" recieved size  "<<small_size<< " in  di ="<<di<<" from pair "<<pair<<"\n";              
            }        
            MPI_Barrier(comm);    
        // 4.2 Send and Recieve buffers 
        // if rank > pair:
            
            if (rank>pair){

                //   Send(small, pair)
                //   Recv(big, pair)
                
                recv_buf.resize(big_size);
                MPI_Send(&small[0], small_size, MPI_INT, pair, 333, comm);
                MPI_Recv(&recv_buf[0], big_size, MPI_INT, pair, 333, comm, &status);
            
                // xi = big + recv    
                big.reserve(big.size() + distance(recv_buf.begin(),recv_buf.end()));
                big.insert(big.end(), recv_buf.begin(), recv_buf.end());
                Xi = big;
            }
        

        // else if rank < pair:
            else{
                //   Send(big, pair)
                //   Recv(small, pair)
                recv_buf.resize(small_size);
                MPI_Send(&big[0], big_size, MPI_INT, pair, 333, comm);
                MPI_Recv(&recv_buf[0], small_size, MPI_INT, pair, 333, comm, &status);
                
                // xi  = small + recv
                small.reserve(small.size() + distance(recv_buf.begin(),recv_buf.end()));
                small.insert(small.end(), recv_buf.begin(), recv_buf.end());
                Xi = small;
            }


        } // for di
        
    // Barrier -------------
    MPI_Barrier(comm);    

    // // 5. Sort locally (final)

    sort(Xi.begin(), Xi.end());


    // std::cout<<"\n X"<<rank<<"  : ";
    // for(auto val: Xi){
    //     std::cout<< val << " ,";
    // }
    // std::cout<<"\n";


} // hyperquick_sort()


#endif // HQ_HPP

