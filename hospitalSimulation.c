#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 
#include <time.h>

/////// size global variables 

#define REGISTRATION_SIZE 10  //

#define RESTROOM_SIZE 10 //

#define  CAFE_NUMBER 10 //

#define  GP_NUMBER 10 //

#define PHARMACY_NUMBER 10 //

#define BLOOD_LAB_NUMBER 10 //

#define  OR_NUMBER 10 //

#define SURGEON_NUMBER 30 //

#define NURSE_NUMBER 30 //

#define SURGEON_LIMIT 5 //

#define NURSE_LIMIT  5 //

#define  PATIENT_NUMBER 1000 //

/////// global hunger and restroom need . IF hunger meter >= hungerValue -> go cafe  ...
// if sleeping times are less and resources are enough there is not much waiting so these lvls (goCafe, goRestroom) must be less to observe cafe and restroom operations
// if recources are less and sleeping times are more , cafe restroom might observing too much so these lvls must be higher to decrease cafe restroom operations

#define goCafe 100 //  variables to decide if patient needs to go cafe or restroom
#define goRestroom 100  // if these higher then observing cafe and restroom will be less  even might not observing

#define  HUNGER_INCREASE_RATE 10 

#define RESTROOM_INCREASE_RATE 10

/////////// time global variables
#define Arrive_Hosp_TIME 20 // Calculate randomly between 1 and given value. IF this value gets higher there will be less waiting 
// because operation waiting times are also small ( milliseconds) so cafe restroom might not be observing


#define  WAIT_TIME 100 // Calculate randomly between 1 and given value.

#define REGISTRATION_TIME  100 // Calculate randomly between 1 and given value.

#define GP_TIME 200 // Calculate randomly between 1 and given value.

#define  PHARMACY_TIME 100 // Calculate randomly between 1 and given value.

#define BLOOD_LAB_TIME 200 // Calculate randomly between 1 and given value.

#define SURGERY_TIME  500 // Calculate randomly between 1 and given value.

#define CAFE_TIME 100 // Calculate randomly between 1 and given value.

#define RESTROOM_TIME  100 // Calculate randomly between 1 and given value.

/////////// cost global variables 

#define  REGISTRATION_COST 100    //

#define  PHARMACY_COST 200 // Calculate randomly between 1 and given value.

#define  BLOOD_LAB_COST 200

#define  SURGERY_OR_COST 200

#define  SURGERY_SURGEON_COST 100

#define  SURGERY_NURSE_COST 50

#define CAFE_COST 200 // Calculate randomly between 1 and given value.

sem_t control ; //  binary semaphore for controlling critical regions instead of mutex

sem_t registerRoom ; 

sem_t OR ;  

sem_t GP; 

sem_t surgeon ; 

sem_t pharmacy ; 

sem_t nurse ; 

sem_t cafe ;  

sem_t restroom ;  

sem_t bloodLab; 

sem_t wait_operation_end;  


int HOSPITAL_WALLET = 0; 

void cafeRestRoom(int id ,int need );

void Pharmacy(int id);

void *patient(void *patient_id);


int main(int argc, char *argv[])
{
      srand(time(NULL)); //  generating  random number purposes
   pthread_t tid[PATIENT_NUMBER];
   int Number[PATIENT_NUMBER];
   int i ;

   for (i = 0; i < PATIENT_NUMBER; i++) {
        Number[i] = i;
    }     

     // initialize semaphores
     sem_init(&control,0,1); // binarry semaphore  instead of mutex for critical regions like Hospital Wallet
     sem_init(&registerRoom, 0, REGISTRATION_SIZE);
     sem_init(&GP,0,GP_NUMBER);
     sem_init(&OR, 0, OR_NUMBER);
     sem_init(&surgeon,0,SURGEON_NUMBER);
     sem_init(&pharmacy,0,PHARMACY_NUMBER);
     sem_init(&nurse,0,NURSE_NUMBER);
     sem_init(&cafe,0,CAFE_NUMBER);
     sem_init(&restroom,0,RESTROOM_SIZE);
     sem_init(&bloodLab,0,BLOOD_LAB_NUMBER);

   for (i = 0; i < PATIENT_NUMBER; i++) {
        pthread_create(&tid[i], NULL, patient, (void *)&Number[i]);  
        usleep(1000*(rand()%Arrive_Hosp_TIME+1)); // patients random millisecond arriving time to the hospital
    }

    // Join each of the threads to wait for them to finish.
    for (i = 0; i < PATIENT_NUMBER; i++) {
        pthread_join(tid[i],NULL);
    } 
    
    printf(" HOSPITAL WALLET TOTAL : %d $" , HOSPITAL_WALLET);
   
    system("PAUSE");   

    return 0;
}

void *patient(void *patient_id) {
     int p_id = *(int *)patient_id; 
     int i ;        
     int Hunger_Meter = rand() % 100 + 1; // Initialized between 1 and 100 at creation.
     int Restroom_Meter = rand() % 100 + 1; ; // Initialized between 1 and 100 at creation
    
     
     printf("Patient %d arrived to Hospital \n ",p_id);
     printf("\n");

     int svalRegister;
     sem_getvalue (&registerRoom, &svalRegister); // if registerRoom sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
     if(svalRegister==0){
          printf("Patient %d waiting an empty desk , Register is full ! \n",p_id);
          printf("\n");
          usleep(1000*(rand()%WAIT_TIME+1)); // if register is full wait for trying again

          Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
          Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
          if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  
                printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                printf("\n");
               cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
               Hunger_Meter = 0; // after Cafe hunger need will reset to 0

               sem_wait(&control);  // controlling critical region
               int cafePurchase = rand() % CAFE_COST +1;
               HOSPITAL_WALLET+= cafePurchase;
               printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
               printf("\n");
               sem_post(&control);
          }
          if(Restroom_Meter>=goRestroom){
               printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
               printf("\n");
               cafeRestRoom(p_id,1);
               Restroom_Meter=0; // after RestRoom  need will reset to 0
          }
        
      }

     sem_wait(&registerRoom);
     printf("Patient %d at Register \n", p_id);
     printf("\n"); // Wait for space to open up in registeration

     sem_wait(&control);
     HOSPITAL_WALLET+=REGISTRATION_COST;
     sem_post(&control);

     usleep(1000*(rand()%REGISTRATION_TIME+1));
     Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
     Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;   
     printf("Patient %d 's registration is done \n ",p_id);
     printf("\n");
     sem_post(&registerRoom); 
     printf("Patient %d left  Register desk \n ",p_id);
     printf("\n");

     int svalGP;
     sem_getvalue (&GP, &svalGP); // if GP sem value = 0 it means full , patient has to wait. Patient's hunger and restroom need will increase . 
     if(svalGP==0){
          printf("Patient %d waiting available GP , all GP's are busy ! \n",p_id);
          printf("\n");
           usleep(1000*(rand()%WAIT_TIME+1));

          Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
          Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
          if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
               printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                printf("\n");
               cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
               Hunger_Meter = 0; // after Cafe hunger need will reset to 0
               sem_wait(&control);
               int cafePurchase = rand() % CAFE_COST +1;
               HOSPITAL_WALLET+= cafePurchase;
               printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
               printf("\n");
               sem_post(&control);
          }
          if(Restroom_Meter>=goRestroom){
              printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
               printf("\n");
               cafeRestRoom(p_id,1);
               Restroom_Meter=0; // after RestRoom  need will reset to 0
          }         
      }

     sem_wait(&GP);      
     printf("Patient %d at GP  \n" , p_id);
     printf("\n");     
     usleep(1000*(rand()%GP_TIME+1));
     Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
     Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
     int gpResult = rand()%3 ; // 0 - pharmacy  , 1 - blood  , 2  surgery
     if(gpResult==0){
          printf("Patient %d examined by GP , needs medicine \n ",p_id);
          printf("\n");
     }
     else if(gpResult==1){
          printf("Patient %d examined by GP , needs blood test \n ",p_id);
          printf("\n");

     }
     else{
          printf("Patient %d examined by GP , needs surgery \n ",p_id);
          printf("\n");

     }
     sem_post(&GP);
      printf("Patient %d left GP office \n ",p_id);
      printf("\n");   
      
      if (gpResult == 0) { // go to pharmacy

         int svalPharmacy;
         sem_getvalue (&pharmacy, &svalPharmacy); // if Pharmacy sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
          if(svalPharmacy==0){
                printf("Patient %d waiting Pharmacy cashiers, all cashiers are busy ! \n",p_id);
                printf("\n");
                usleep(1000*(rand()%WAIT_TIME+1));

             Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
             Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
               if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                    printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                    printf("\n");
                      cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                      Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                      sem_wait(&control);
                      int cafePurchase = rand() % CAFE_COST +1;
                      HOSPITAL_WALLET+= cafePurchase;
                      printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
                      printf("\n");
                      sem_post(&control);
               }
              if(Restroom_Meter>=goRestroom){
                  printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                  printf("\n");
                    cafeRestRoom(p_id,1);
                    Restroom_Meter=0; // after RestRoom  need will reset to 0
               }
               
          }

           Pharmacy(p_id); // pharmacy function
           Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
           Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
           sem_wait(&control);
           int pharmacyPurchase = rand() % PHARMACY_COST +1;
           HOSPITAL_WALLET+= pharmacyPurchase;
           printf("Patient %d spent %d $ at the Pharmacy \n",p_id,pharmacyPurchase);
            printf("\n");
           sem_post(&control);           

      }
      else if (gpResult == 1) { // go to blood Lab
              int svalBloodLab;
              sem_getvalue (&bloodLab, &svalBloodLab); // if BloodLab sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalBloodLab==0){
                   printf("Patient %d waiting Blood Lab stations , all assistants are busy ! \n",p_id);
                   printf("\n");
                   
                   usleep(1000*(rand()%WAIT_TIME+1));

                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
                   if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                         printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                         printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                        int cafePurchase = rand() % CAFE_COST +1;
                        HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }   
                   if(Restroom_Meter>=goRestroom){
                        printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                        printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }
                     
               }
          sem_wait(&bloodLab);
          printf("Patient %d gave Blood at Blood Lab \n", p_id);
          printf("\n");

          sem_wait(&control);
          HOSPITAL_WALLET+= BLOOD_LAB_COST;
          sem_post(&control);

          usleep(1000*(rand()%BLOOD_LAB_TIME+1));
          Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
          Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
          sem_post(&bloodLab);
           printf("Patient %d left Blood Lab \n ",p_id);
           printf("\n");    
          // Go back to GP get medicine or not
              int svalGPback;
              sem_getvalue (&GP, &svalGPback); // if GP sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalGPback==0){
                    printf("Patient %d waiting available GP for Blood Results , all GP's are busy ! \n",p_id);
                    printf("\n");
                    usleep(1000*(rand()%WAIT_TIME+1));                    
                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
                   if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                          printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                          printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                         int cafePurchase = rand() % CAFE_COST +1;
                         HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }   
                   if(Restroom_Meter>=goRestroom){
                         printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                         printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }
                    
               }
           sem_wait(&GP);      
           printf("Patient %d at GP  for Blood Results \n" , p_id);
           printf("\n");
           usleep(1000*(rand()%GP_TIME+1));  
           Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
           Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
           int gpResultBlood = rand()%2 ; // 0  leave ,  1 pharmacy
           if(gpResultBlood== 1){
                printf("Patient % d needs medicine as result of blood test \n ",p_id);
                printf("\n");
           }
           else {
                printf("Patient %d doesnt need medicine as result of blood test \n ",p_id);
                printf("\n");
            }                  
           
           sem_post(&GP);
            printf("Patient %d left GP office \n ",p_id);
            printf("\n");         
           if(gpResultBlood == 1){           
            // go back pharmacy
             int svalPharBack;
              sem_getvalue (&pharmacy, &svalPharBack); // if Pharmacy sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalPharBack==0){
                   printf("Patient %d waiting Pharmacy cashiers, all cashiers are busy ! \n",p_id);
                   printf("\n");
                   
                    usleep(1000*(rand()%WAIT_TIME+1));

                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
                   if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                         printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                         printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                        int cafePurchase = rand() % CAFE_COST +1;
                        HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }   
                   if(Restroom_Meter>=goRestroom){
                         printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                         printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }
                    
               }
              Pharmacy(p_id); // pharmacy function
               Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
               Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
              sem_wait(&control);
              int pharmacyPurchase = rand() % PHARMACY_COST +1;
              HOSPITAL_WALLET+= pharmacyPurchase;
              printf("Patient %d spent %d $ at the Pharmacy \n",p_id,pharmacyPurchase);
               printf("\n");
              sem_post(&control);
                     
           }                   

      }
      else if (gpResult == 2){ // go to OR
              int svalOR;
              sem_getvalue (&OR, &svalOR); // if OR sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalOR==0){
                   printf("Patient %d waiting an empty OR, all ORs are full ! \n",p_id);
                   printf("\n");
                   usleep(1000*(rand()%WAIT_TIME+1));

                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
                   if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                         printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                         printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                        int cafePurchase = rand() % CAFE_COST +1;
                        HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d  spent %d $ at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }   
                   if(Restroom_Meter>=goRestroom){
                          printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                          printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }
                     
               }
              sem_wait(&OR);
              printf("Patient %d at OR , waiting doctors and nurses  \n", p_id) ;
              printf("\n");
         
               int surgeonNeeded = rand() % SURGEON_LIMIT +1;
               
               for(i = 0; i<surgeonNeeded;i++){  // i assume that when Patient at OR cant go for cafe and restroom so he must wait till Operation start and end 
                  sem_wait(&surgeon);
                  
               }        
               usleep(1000*(rand()%WAIT_TIME+1)); // wait surgeons to arrive OR 
               Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
               Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;

               int nurseNeeded = rand() % NURSE_LIMIT +1;
               printf("Patient %d needs %d surgeons , %d nurses for the surgery \n",p_id,surgeonNeeded,nurseNeeded);
               printf("\n");
               for(i = 0; i<nurseNeeded;i++){
                sem_wait(&nurse);
               }
               usleep(1000*(rand()%WAIT_TIME+1)); // wait nurses to arrive OR 
               Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
               Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
         
              printf("Patient % d's surgery starts , everyone is ready ! \n " ,p_id);
              printf("\n");
              usleep(1000*(rand()%SURGERY_TIME+1));

              for(i = 0; i<surgeonNeeded;i++){
                   sem_post(&surgeon);
               }
         
             for(i = 0; i<nurseNeeded;i++){
                  sem_post(&nurse);
               }
               int surgeryCost = SURGERY_OR_COST + (surgeonNeeded * SURGERY_SURGEON_COST) + (nurseNeeded * SURGERY_NURSE_COST);
               sem_wait(&control);
              HOSPITAL_WALLET+= surgeryCost;
              sem_post(&control);
             printf("Patient %d 's Surgery Done  doctors and nurses left the OR \n" , p_id);
             printf("\n");             
             sem_post(&OR); 
              printf("Patient %d left OR  \n ",p_id);
              printf("\n");        

            // Go back to GP , got medicine or not
            int svalGpAfterOR;
              sem_getvalue (&GP, &svalGpAfterOR); // if GP sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalGpAfterOR==0){
                    usleep(1000*(rand()%WAIT_TIME+1));
                    printf("Patient %d waiting available GP after Surgery , all GP's are busy ! \n",p_id);
                    printf("\n");

                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;
                   if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                         printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                         printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                        int cafePurchase = rand() % CAFE_COST +1;
                        HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d , spent %d at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }   
                   if(Restroom_Meter>=goRestroom){
                        printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                        printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }                    
               }
           sem_wait(&GP);      
           printf("Patient %d at GP after Surgery \n" , p_id);
           printf("\n");
           usleep(1000*(rand()%GP_TIME+1));
            Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
            Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
           int gpResultSurgery = rand()%2 ; // 0  doesnt need medicine ,  1 need medicine
           if(gpResultSurgery==1){
                printf("Patient %d needs medicine after Surgery \n",p_id);
                printf("\n");
           }
           else{
                printf("Patient %d doesnt need medicine after Surgery \n ",p_id);
                printf("\n");
           }
           sem_post(&GP);
            printf("Patient %d left GP office \n ",p_id);
            printf("\n");    
           if(gpResultSurgery == 1){           
              // go back to pharmacy
              int svalPharAfterOR;
              sem_getvalue (&pharmacy, &svalPharAfterOR); // if pharmacy sem value = 0 it means full , has to wait. Patient's hunger and restroom need will increase . 
              if(svalPharAfterOR==0){
                   usleep(1000*(rand()%WAIT_TIME+1));
                   printf("Patient %d waiting Pharmacy cashiers, all cashiers are busy ! \n",p_id);
                   printf("\n");                
                   
                        Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
                        Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;                       
                    if(Hunger_Meter>=goCafe){ // if reaches 100 Patient can go Cafe to eat  ,
                         printf("Patient %d's hunger need is full ! Needs to eat \n",p_id);
                         printf("\n");
                        cafeRestRoom(p_id,0); //  0 parameter means patient needs to go CAFE , 1 is Restroom                        
                        Hunger_Meter = 0; // after Cafe hunger need will reset to 0
                        sem_wait(&control);
                        int cafePurchase = rand() % CAFE_COST +1;
                        HOSPITAL_WALLET+= cafePurchase;
                        printf("Patient %d , spent %d at the Cafe \n",p_id,cafePurchase);
                        printf("\n");
                        sem_post(&control);
                    }                       
                   if(Restroom_Meter>=goRestroom){
                           printf("Patient %d's restroom need is full ! Needs to go restroom \n",p_id);
                           printf("\n");
                       cafeRestRoom(p_id,1);
                       Restroom_Meter=0; // after RestRoom  need will reset to 0
                    }                    
               }
               Pharmacy(p_id);
               Hunger_Meter+= rand() % HUNGER_INCREASE_RATE +1;
               Restroom_Meter+= rand() % RESTROOM_INCREASE_RATE+1 ;  
               sem_wait(&control);
               int pharmacyPurchase = rand() % PHARMACY_COST +1;
               HOSPITAL_WALLET+= pharmacyPurchase;
               printf("Patient %d spent %d $ at the Pharmacy \n",p_id,pharmacyPurchase);
               printf("\n");
               sem_post(&control);
                 
           }                        
         
      }
        
     printf("Patient %d left the HOSPITAL \n" , p_id);
     printf("\n");

}

void cafeRestRoom (int id, int need ){
     int p_id = id;
     int needType = need ;
     if(needType==0){ // Patient needs Cafe 
       int svalCafe;
        sem_getvalue (&cafe, &svalCafe); // if cafe's sem value = 0 it means full , has to wait. 
        if(svalCafe==0){
             printf("Patient %d waiting an available cashier at Cafe , all Cashiers are busy ! \n",p_id);
             printf("\n");
           usleep(1000*(rand()%WAIT_TIME+1)); // since this function called because patient's hunger or restroom need is already 100 no need to increase need lvls
        }
       sem_wait(&cafe);
       printf("Patient %d at Cafe \n",p_id);
       printf("\n");
       usleep(1000*(rand()%CAFE_TIME+1));
       printf("Patient %d bought something from Cafe \n",p_id);
       printf("\n");
       sem_post(&cafe);
       printf("Patient %d left the Cafe \n ",p_id);
       printf("\n");

     }
     else { // Patient needs Restroom
     int svalRestroom;
        sem_getvalue (&restroom, &svalRestroom); // if restroom's sem value = 0 it means full , has to wait. 
        if(svalRestroom==0){
             printf("Patient %d waiting an available Restroom , all Restrooms are full ! \n ",p_id);
             printf("\n");
           usleep(1000*(rand()%WAIT_TIME+1)); // since this function called because patient's hunger or restroom need is already 100 no need to increase need lvls
        }
      sem_wait(&restroom);
      printf("Patient %d at Restroom \n ",p_id);
      printf("\n");
      usleep(1000*(rand()%RESTROOM_TIME+1));
      sem_post(&restroom);
      printf("Patient %d left the Restroom \n",p_id);
      printf("\n");
     }    
}
void Pharmacy(int id ){
    int p_id = id;
            sem_wait(&pharmacy);
            printf("Patient %d at Pharmacy \n", p_id);
            printf("\n");
            usleep(1000*(rand()%PHARMACY_TIME+1));            
            printf("Patient % d bought medicine \n", p_id);
            printf("\n");            
            sem_post(&pharmacy); 
            printf("Patient %d left Pharmacy \n",p_id);
            printf("\n"); 
}