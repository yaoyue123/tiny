/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static void nameError(TreeNode * t, char * message)
{
  fprintf(listing,"Name error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case IntK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, so treat as new definition */
            st_insert(t->attr.name,t->lineno,location++,INT);
          else
          /* already in table, redeclaration */
            nameError(t, "redeclaration of identifier");
          break;
        case CharK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, so treat as new definition */
            st_insert(t->attr.name,t->lineno,location++,CHAR);
          else
          /* already in table, redeclaration */
            nameError(t, "redeclaration of identifier");
          break;
        case AssignK:
        case ReadK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, undeclared */
            nameError(t, "identifier undeclared");
          else
          /* already in table, so ignore location, 
             add line number of use only */ 
            st_insert(t->attr.name,t->lineno,0,ID);
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, undeclared */
            nameError(t, "identifier undeclared");
          else
          /* already in table, so ignore location, 
             add line number of use only */ 
            st_insert(t->attr.name,t->lineno,0,ID);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if (!(((t->child[0]->type == Integer) && (t->child[1]->type == Integer)) ||
              ((t->child[0]->type == Character) && (t->child[1]->type == Character)) ||
              (t->child[0]->type == Constant) ||
              (t->child[1]->type == Constant))) 
            typeError(t,"Op applied to conflict types");
          if ((t->attr.op == EQ) || (t->attr.op == LT))
            t->type = Boolean;
          else if (t->child[0]->type == Integer || t->child[1]->type == Integer)
            t->type = Integer;
          else if (t->child[0]->type == Character || t->child[1]->type == Character)
            t->type = Character;
          else
            t->type = Constant;
          break;
        case ConstK:
          t->type = Constant;
          break;
        case IdK: {
          TokenType type = st_looktype(t->attr.name);
          switch (type)
          {
            case INT:
              t->type = Integer;
              break;
            case CHAR:
              t->type = Character;
              break;
            default:
              typeError(t,"Unknown type");
              break;
          }
          break;
        }
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type != Boolean)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK: {
          TokenType type = st_looktype(t->attr.name);
          switch(type) {
            case INT:
              if (t->child[0]->type != Integer && t->child[0]->type != Constant)
                typeError(t->child[0],"assignment of non-integer value");
              break;
            case CHAR:
              if (t->child[0]->type != Character && t->child[0]->type != Constant)
                typeError(t->child[0],"assignment of non-character value");
              break;
            default:
              typeError(t,"Unknown type");
              break;
          }
          break;
        }
        case WriteK:
          if (t->child[0]->type != Integer && t->child[0]->type != Character && t->child[0]->type != Constant)
            typeError(t->child[0],"write of neither integer nor character value");
          break;
        case RepeatK:
          if (t->child[1]->type != Boolean)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
