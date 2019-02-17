#include "predicate.h"
#include "utility.h"

Output* generateFinalOutput(PredicateFilter* pf)
{
    Output* buffer = createOutput();  //temporary buffer
    Output* output = createOutput();   //final results
    PredicateStack ps;
    initPredicateStack(&ps);

    //get input 2-tuple list
    TupleList* tl = pf->tuple_list;
    int start_index = 0;
    int end_index = getTupleListSize(tl);   
    //iterate through each element in tuple list
    for(int i = start_index; i < end_index; i++)
    {
        TupleElement tl_ele = getTupleListElement(tl, i);
        int state = tl_ele.state;  //query state
        char* text = tl_ele.text;  //text content
        //starting position of an object
        if(strcmp(text, "{")==0)
        {
            PredicateStackElement ps_ele;
            ps_ele.predicate_state = state;
            ps_ele.first_candidate_pos = getOutputSize(buffer);
            pushPredicateStack(&ps, ps_ele);
        }
        //ending position of an object, needs to evaluate predicate logics
        else if(strcmp(text, "}")==0)
        {
            PredicateStackElement ps_ele = getTopPredicateStack(&ps);
            int pred_state = ps_ele.predicate_state;
            PredicateStateInfo ps_info = pf->state_mapping[pred_state];
            ASTNode* root = ps_info.sub_tree; 
            //evaluate predicate logics 
            JSONPathValue v;// = jpe_Evaluate(root, ps_info.con_state_list);
            int first_idx = ps_ele.first_candidate_pos;
            int buf_size = getOutputSize(buffer);
            int rmv_num = buf_size - first_idx;
            if (v.boolean)
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
            // clearKeyValuePair(ps_info.con_state_list);
            popPredicateStack(&ps); 
        }
        else if(getPredicateStackSize(&ps)>0)
        {
            PredicateStackElement ps_ele = getTopPredicateStack(&ps);
            int pred_state = ps_ele.predicate_state;
            PredicateStateInfo ps_info = pf->state_mapping[pred_state];
            JSONPathKeyValuePair* con_state_list = ps_info.con_state_list;
            int num_con_state = ps_info.num_con_state;
            int index;
            for(index = 0; index<num_con_state; index++)
            {   
                if(con_state_list[index].state==state) break;
            }
            //update value for predicate conditions
            if(index<num_con_state)
            {  
                if(text!=NULL&&(text[0]=='"'||text[0]=='{'||text[0]=='['))
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
                }
            }
            else //candidate output
            {
                addOutputElement(buffer, text);
            }
        }
        else addOutputElement(output, text); //final output 
    }

    freeOutput(buffer);
    return output;
}
