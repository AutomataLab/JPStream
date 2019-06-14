#include "predicate.h"
#include "utility.h"

Output* generateFinalOutput(PredicateFilter* pf)
{ 
    char* input_stream = pf->input_stream;
    Output* output = pf->output;  //final results
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
           int start_pos = tuple.start_position;
           int end_pos = tuple.end_position;
           char* text_value = substring(input_stream, start_pos, end_pos);
           addOutputElement(output, text_value);
       }
       return output; 
    }

    if(pf->buffer==NULL) pf->buffer = createOutput();
    Output* buffer = pf->buffer;   //temporary buffer 
    PredicateStack ps = pf->pstack;

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
                        char* text_value = getOutputElement(buffer, j); 
                        addOutputElement(output, text_value); 
                        resetOutputElement(buffer, j);
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
                char* text_value = NULL;
                int start_pos = tuple.start_position;
                if(start_pos!=-1)
                {
                    int end_pos = tuple.end_position;
                    text_value = substring(input_stream, start_pos, end_pos); 
                }
                else
                {
                    text_value = allocate_and_copy(text); 
                }
                pc[index].text = text_value;       
            }
            else //add candidate output into buffer
            {
                int start_pos = tuple.start_position;
                int end_pos = tuple.end_position;
                char* text_value = substring(input_stream, start_pos, end_pos);
                addOutputElement(buffer, text_value); 
            }
        }
    }
    pf->pstack = ps;
    //printf("stack size %d output size %d\n", getPredicateStackSize(&pf->pstack), getOutputSize(output));
    ///printf("buffer size %d %s\n", getOutputSize(&buffer), getOutputElement(&buffer, 0));
    return output;
}
