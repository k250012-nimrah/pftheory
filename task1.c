#include <stdio.h>
#include <string.h>
void addBook(int isbns[],char titles[][50],float prices[],int quantities[],int *count){
    int isbn,i,found=0;
    if(*count>=100)return;
    printf("Enter ISBN: ");
    scanf("%d",&isbn);
    for(i=0;i<*count;i++){
        if(isbns[i]==isbn){
            found=1;
            break;
        }
    }
    if(found){
        printf("Book already exists\n");
        return;
    }
    isbns[*count]=isbn;
    printf("Enter title: ");
    scanf(" %[^\n]",titles[*count]);
    printf("Enter price: ");
    scanf("%f",&prices[*count]);
    printf("Enter quantity: ");
    scanf("%d",&quantities[*count]);
    (*count)++;
}
void processSale(int isbns[],int quantities[],int count){
    int isbn,i,num,found=0;
    printf("Enter ISBN: ");
    scanf("%d",&isbn);
    for(i=0;i<count;i++){
        if(isbns[i]==isbn){
            found=1;
            printf("Enter number of copies sold: ");
            scanf("%d",&num);
            if(num>quantities[i])printf("Out of stock\n");
            else quantities[i]-=num;
            break;
        }
    }
    if(!found)printf("Book not found\n");
}
void lowStock(int isbns[],char titles[][50],float prices[],int quantities[],int count){
    int i,found=0;
    for(i=0;i<count;i++){
        if(quantities[i]<5){
            printf("ISBN: %d Title: %s Price: %.2f Quantity: %d\n",isbns[i],titles[i],prices[i],quantities[i]);
            found=1;
        }
    }
    if(!found)printf("No low stock books\n");
}

int main(){
    int isbns[100],quantities[100],count=0,choice;
    char titles[100][50];
    float prices[100];
    while(1){
        printf("1.Add New Book\n2.Process Sale\n3.Low Stock Report\n4.Exit\nEnter choice: ");
        scanf("%d",&choice);
        switch(choice){
            case 1:addBook(isbns,titles,prices,quantities,&count);break;
            case 2:processSale(isbns,quantities,count);break;
            case 3:lowStock(isbns,titles,prices,quantities,count);break;
            case 4:return 0;
            default:printf("Invalid choice\n");
        }
    }
}
