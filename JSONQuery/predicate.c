#include "predicate.h"
#include "utility.h"

Output* generateFinalOutput(PredicateFilter* pf)
{
    Output* output = createOutput();  //final results
    //get input 2-tuple list
    TupleList* tl = pf->tuple_list;
    int start_index = 0;
    int end_index = getTupleListSize(tl);
    //check whether it has predicates
    int s;
    for(s = 0; s < MAX_PREDICATE_STATE; s++)
        if(pf->condition_list[s]!=NULL) break;
    //there is no predicates, no need to check
    if(s==MAX_PREDICATE_STATE)
    {
       for(int i = start_index; i < end_index; i++) 
       {
           Tuple tuple = getTuple(tl, i);  
           addOutputElement(output, tuple.text);
       }
       return output; 
    }
    Output* buffer = createOutput();   //temporary buffer
    PredicateStack ps;
    initPredicateStack(&ps);

    //get input 2-tuple list
    /*TupleList* tl = pf->tuple_list;
    int start_index = 0;
    int end_index = getTupleListSize(tl);   */
    //iterate through each element in tuple list
    for(int i = start_index; i < end_index; i++)
    {
        Tuple tuple = getTuple(tl, i);
        int state = tuple.state;  //query state
        char* text = tuple.text;  //text content
        //starting position of an object
        if(strcmp(text, "{")==0)
        {
            PredicateStackElement ps_elt;
            ps_elt.predicate_state = state;
            ps_elt.output_buffer_pointer = getOutputSize(buffer);
            pushPredicateStack(&ps, ps_elt);
        }
        //ending position of an object, needs to evaluate predicate logics
        else if(strcmp(text, "}")==0)
        {
            PredicateStackElement ps_elt = getTopPredicateStack(&ps);
            int pred_state = ps_elt.predicate_state;
            PredicateCondition* condition_list = pf->condition_list[pred_state]; 
            ASTNode* node = getContextSubtree(pf->ctx, pred_state);
            //PredicateStateInfo ps_info = pf->state_mapping[pred_state];
            //ASTNode* root = ps_info.sub_tree; 
            //evaluate predicate logics 
            int index = 0;
            /*while(true)
            {
                if(condition_list[index].name!=NULL){
                    printf("name is %s value is %s\n", condition_list[index].name, condition_list[index].text);
                }
                else break;
                index++;
            }*/
            bool v = evaluateExpression(node, condition_list); //printf("evaluate %d\n", v);
            int first_idx = ps_elt.output_buffer_pointer;
            int buf_size = getOutputSize(buffer);
            int rmv_num = buf_size - first_idx;
            if (v)
            {
                //release buffer into final output list
                if(getPredicateStackSize(&ps)==1)
                {
                    for(int j = first_idx; j<buf_size; j++)
                    { 
                        addOutputElement(output, getOutputElement(buffer, j)); 
                    }
                    //clear the last few elements into buffer
                    removeOutputElement(buffer, rmv_num);
                }
            }  
            //clear the last few elements into buffer
            else removeOutputElement(buffer, rmv_num); 
            //set the value of evaluate table to default value
            clearKeyValuePair(condition_list);
            popPredicateStack(&ps); 
        }
        else if(getPredicateStackSize(&ps)>0)
        {
            PredicateStackElement ps_elt = getTopPredicateStack(&ps);
            int pred_state = ps_elt.predicate_state;
            PredicateCondition* condition_list = pf->condition_list[pred_state];
           
            //PredicateStateInfo ps_info = pf->state_mapping[pred_state];
            //JSONPathKeyValuePair* con_state_list = ps_info.con_state_list;
            //int num_con_state = ps_info.num_con_state;
            //check whether the current state is a predicate condition
            ASTNode* node = getContextSubtree(pf->ctx, state); ///printf("state is %d\n", state);
            int index = 0;
            while(node!=NULL)
            //for(index = 0; index<num_con_state; index++)
            {  
                if(condition_list[index].name==NULL) break;
                 
                if(strcmp(condition_list[index].name, node->string)==0) break;
                index++;
                //if(con_state_list[index].state==state) break;
            }
            //update value for predicate conditions
            if(node!=NULL&&condition_list[index].name!=NULL)
            //if(index<num_con_state)
            {  
                condition_list[index].text = allocate_and_copy(text);//substring(text,0, strlen(text)); ///printf(">>>name is %s text is %s\n", condition_list[index].name, condition_list[index].text);
                /*if(text!=NULL&&(text[0]=='"'||text[0]=='{'||text[0]=='['))
                {
                    con_state_list[index].value.vtype = jvt_string; 
                    if(text[0]=='"') text = substring(text, 1, strlen(text)-1); 
                    con_state_list[index].value.string = text; 
                }
                else if(text!=NULL&&(strcmp(text,"true")==0||strcmp(text,"")==0))
                {
                    con_state_list[index].value.vtype = jvt_boolean;
                    con_state_list[index].value.boolean = true;
                }
                else if(text!=NULL&&strcmp(text,"false")==0)
                {
                    con_state_list[index].value.vtype = jvt_boolean;
                    con_state_list[index].value.boolean = false;
                }
                else if(text!=NULL&&strcmp(text,"null")==0)
                {
                    con_state_list[index].value.vtype = jvt_null; 
                }
                else if(text!=NULL)
                {
                    con_state_list[index].value.vtype = jvt_number;
                    con_state_list[index].value.number = atof(text); 
                }*/
            }
            else //candidate output
            {
                addOutputElement(buffer, text);
            }
        }
        //else addOutputElement(output, text); //final output 
    }

    freeOutput(buffer);
    return output;
}
