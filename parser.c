char command[50];

int parser(){
        print("Welcome team 11 OS FileSystem\n");
        print("input 'help' get more information\n\n\n");
        startsys();
        int len;
        while(1)
        {
                cout<<currentdir+">";
                cin>>cmd;
                if(cmd=="help"){
                        help();
                }
                else if(cmd=="dir"){
                        cin>>command;
                        dir(command);
                }
                else if(cmd=="tree"){
                        cin>>command;
                        tree(command);
                }
                else if(cmd=="find"){
                        cin>>command;
                        find(command);
                }
                else if(cmd=="cd"){
                }
                else if(cmd=="type"){
                }
                else if(cmd=="touch"){
                }
                else if(cmd=="del"){
                }
                else if(cmd=="edit"){
                }
                else if(cmd=="append"){
                }
                else if(cmd=="copy"){
                }
                else if(cmd=="quit"){
                }
                else {
                        printf("The cmd is not exits!\nplease enter help to get right cmds.\n");
                }
        }
}                                                                                                                                                                           59,1         底端
