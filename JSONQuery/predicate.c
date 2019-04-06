#include "predicate.h"
#include "utility.h"

Output* generateFinalOutput(PredicateFilter* pf)
{
    Output* output = createOutput();  //final results
    //get 2-tuple list
    TupleList* tl = pf->tuple_list;
    int start_index = 0;
    int end_index = getTupleListSize(tl);
    //check whether current query has any predicates
    int s;
    for(s = 0; s < MAX_PREDICATE_STATE; s++)
        if(pf->predicate_conditions[s]!=NULL) break;
    //there is no predicates, simply add each text into final output
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
        //ending position of an object, needs to evaluate predicate conditions
        else if(strcmp(text, "}")==0)
        {
            PredicateStackElement ps_elt = getTopPredicateStack(&ps);
            int pred_state = ps_elt.predicate_state;
            PredicateCondition* pc = pf->predicate_conditions[pred_state];
            ASTNode* node = getContextSubtree(pf->ctx, pred_state);
            //evaluate predicate conditions 
            bool v = evaluateExpression(node, pc); 
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
            clearPredicateCondtion(pc);
            popPredicateStack(&ps); 
        }
        else if(getPredicateStackSize(&ps)>0)
        {
            PredicateStackElement ps_elt = getTopPredicateStack(&ps);
            int pred_state = ps_elt.predicate_state;
            PredicateCondition* pc = pf->predicate_conditions[pred_state];
        
            //check whether the current state is a predicate condition
            ASTNode* node = getContextSubtree(pf->ctx, state);
            int index = 0;
            while(node!=NULL)
            {  
                if(pc[index].name==NULL) break;
                if(strcmp(pc[index].name, node->string)==0) break;
                index++;
            }
            //update value for predicate condition
            if(node!=NULL&&pc[index].name!=NULL)
            {  
                pc[index].text = allocate_and_copy(text); 
            }
            else //add candidate output into buffer
            {
                addOutputElement(buffer, text);
            }
        }
    }

    freeOutput(buffer);
    return output;
}
