#ifndef _slb_
#define _slb_

// return 1 if en elem is __ in the __ array (similar to Python's in() )
int in_the(int array[];int member;){
  foreach (int num; array) {
    if( num == member ) return 1;}
  return 0;
}

// same just overloading for strings
int in_the(string array[];string member;){
  foreach (string num; array) {
    if( num == member ) return 1;}
  return 0;
}

// append if not in
int append_if_not_in(int array[];int member;) {
  if( !in_the(array,member ) ){
    push (array,member);
    return 1;
  }
  else return 0;
}

int append_if_not_in(string array[];string member;){
  if ( !in_the (array,member )) {
    push (array,member);
    return 1;
  }
  else  {
    return 0;
  }}

  // returns the class for adjacent edge
int get_hedge_adj_prim_class (int hedge){
  int equiv = hedge_nextequiv(0, hedge); // only one for flat surface
  int pr = hedge_prim(0, equiv) ;
  return prim(0,"class",pr);
 }

  // return cubic interpolation
vector target( vector P0; vector P1; vector P2; vector P3; float t ){
  if (t > 1) t = 1;
  if (t < 0) t = 0;
  float b1 = (1-t) ;
  vector target1 = pow(b1,3)*P0 + 3*pow(b1,2)*t*P1 + 3*b1*pow(t,2)*P2 + pow(t,3)*P3;
  return target1;
}

// generates poly and sets up the class inherited from parents
int generate_poly(int pt1; int pt2; int pt3; int prim_num){

  int new_prim2 = addprim(0, "poly", pt1, pt2, pt3);
  vector cd = prim (0,"Cd",prim_num);
  int class = prim (0,"class",prim_num);
  setprimattrib(0,"Cd",new_prim2,cd );
  setprimattrib(0,"__class",new_prim2,class);
  setprimattrib(0,"parent_prim",new_prim2,prim_num);

  return new_prim2;
}

//build a polyline between prims
int build_the_link_4(string prim_pair){
  string b[] = split(prim_pair,"_");
  int first = atoi(b[0]);
  int second = atoi(b[1]);
  vector p1 = prim(0, "P", first);
  vector p2 = prim(0, "P", second);
  int pt1 = addpoint(0, p1);
  int pt2 = addpoint(0, p2);
  int pr = addprim(0, "polyline", pt1, pt2);
  setpointattrib( 0, "prim", pt1, first );
  setpointattrib( 0, "prim", pt2, second );
  return pr;
}

// build an array of unique values from geo(prim, ) attrs similar to promote to detail
int [] build_set_from_prim_attr (string attr_name){
  int array[]={};
  for (int pr=0; pr < nprimitives(0); pr ++ ){
    int val = prim(0, attr_name  , pr);
    append_if_not_in(array, val);
  }
  return array;
}

// there is always uber class based on its mass
int find_uber_class(int class; int sorted_classes[]){
    int members_count[] = findattribval(0,"prim", "class",class );
    // int nei_classes[]={};
    int nei_classes_ids[]={}; // list of unique values per class
    int count_mem = len(members_count);
    foreach(int pr; members_count){
      int neighbours[] = prim(0,"neighbours",pr);
      foreach (int neighbour; neighbours){
        int nei_class = prim ( 0, "class", neighbour );
        if (nei_class!=class && !in_the (nei_classes_ids, nei_class)) append( nei_classes_ids, nei_class );
      }
    }
    int the_class=-1;
    /* int count1=0; */
    /* int max_score = 0; */
    /* foreach (int class_id; nei_classes_ids){ */
    /*   int count = 0; */
    /*   foreach (int nc;  nei_classes ) { */
    /*     if ( class_id == nc ) count++; */
    /*   } */
    /*   max_score = max( count, max_score ); */
    /*   if ( count >= max_score )  {the_class = class_id;count1=count;} */
    /* } */

    // the dominant class will be the last because the array is sorted
      foreach (int class_; sorted_classes){
        if (in_the( nei_classes_ids,class_ )) the_class=class_;
    }

    return the_class;
}

string [] point_pairs( int primnum){
  int hedge0 = primhedge(0, primnum);
  int hedge1 = hedge_next(0, hedge0);
  int hedge2 = hedge_next(0, hedge1);
  int class = prim(0,"class", primnum);
  string point_pairs[] = {};
  int hedges[] = array(hedge0,hedge1,hedge2);
  foreach (int hedge; hedges){
    int adjacent_class =  get_hedge_adj_prim_class (hedge);
    if (adjacent_class != class) {
      int src_pt = hedge_srcpoint(0, hedge);
      int dst_pt = hedge_dstpoint(0, hedge);
      string pair = sprintf("%s_%s", min( src_pt,dst_pt ),max( src_pt,dst_pt )  );
      append(point_pairs, pair);
    }}
  return point_pairs;
}

int check_multiple_classes(int pt){
  int classes[]= {};
  foreach (int prim; pointprims(0, pt)){
    int class = prim(0,"class",prim);
    if (!in_the (classes,class)) push(classes,class);
  }
  if (len(classes)>2) return 1;
  else return 0;
}

int get_next_point( int point1; int point2  ){
  int pt0=-1;
  string geo_pairs[]=detail(0,"geo_pairs");
  //string classes =  point(0,"classes",point1);
  int neighbours[]= neighbours(0, point1);
  foreach( int pt; neighbours ){
      string pair = sprintf("%s_%s",min(pt,point1),max( pt,point1 ));
      if (pt != point2 && in_the(geo_pairs,pair)) pt0 = pt;
  }
  return pt0;
}

// cubic interpolated fan
int generate_fan( string pt_pair; int primnum; int base_pt ;int numdiv ;float linear ){
  // returns 1 if success and interpolation possible
  int points[]={};
  string b[] = split(pt_pair,"_");
  int first = atoi(b[0]);
  int second = atoi(b[1]);
  push(points,first);
  push(points,second);

  if ( base_pt == -1){
    foreach( int pnt; primpoints (0,primnum) ){
      if (pnt!=first && pnt!=second) base_pt=pnt;
  }}
  int pt0 = get_next_point( first, second );
  int pt3 = get_next_point( second, first );
  // let's interpolate one edge more back and forth
  int pt4 = get_next_point( pt3, second );
  int pt_minus = get_next_point( pt0, first );
  int pt_current = first;

  vector P0 = point(0,"P",pt0);
  vector P1 = point(0,"P",first);
  vector P2 = point(0,"P",second);
  vector P3 = point(0,"P",pt3);
  vector P_minus = point(0,"P",pt_minus);
  vector P4 = point(0,"P",pt4);
  for (int i = 0; i<= numdiv; i++)    {
    float bias = float(i)/numdiv/3+1/3.0;
    float bias0 = float(i)/numdiv; // for linear from 0.0 to 1.0
    float bias1 = bias+1/3.0; // bias for blending with first half
    float bias2 = bias-1/3.0; // bias for blending with second half

    if ( pt_minus ==-1 || pt4 ==-1 || pt0 ==-1 || pt3 ==-1  ) {
      int new_prim1 = generate_poly ( base_pt,second,pt_current,primnum);
      return 0;
    }
    vector target1 = target(P0, P1, P2, P3, bias );
    vector half1 = target(P_minus, P0, P1, P2, bias1 );
    vector half2 = target(P1, P2, P3, P4, bias2);

    vector target_first_half =  lerp(half1, target1, bias );
    vector target_second_half =  lerp(target1, half2 ,bias );

    vector target_final = lerp(target_first_half, target_second_half ,bias );
    vector target0 = lerp(P2, P1, 1-bias0); // lerp f

    vector target = lerp( target0, target_final , linear);

    if (i==0) setpointattrib(0,"P", first,target );
    if (i==numdiv) setpointattrib(0,"P", second,target );

    int newPt = addpoint(0,target);
    setpointgroup(0,"edge",newPt,1);

    //int new_prim0 = addprim(0, "poly", base_pt,newPt,pt_current);
    int new_prim0 = generate_poly ( base_pt,newPt,pt_current,primnum);
    pt_current = newPt;
    setprimgroup(0,"subdivide",new_prim0,1);
    removeprim(0,primnum,1);

    }
  return 1;
}

// linear interpolated fan
int generate_linear_fan ( string pair; int primnum; int base_pt ;int numdiv){
  int points[]={};
  string b[] = split(pair,"_");
  int first = atoi(b[0]);
  int second = atoi(b[1]);
  push(points,first);
  push(points,second);
  if ( base_pt == -1){
    foreach( int pnt; primpoints (0,primnum) ){
      if (pnt!=first && pnt!=second) base_pt=pnt;
  }}
  int pt_current = first;
  vector P1 = point(0,"P",first);
  vector P2 = point(0,"P",second);
  for (int i = 0; i<= numdiv; i++)
    {
      float bias = float(i)/numdiv; // for linear from 0.0 to 1.0
      vector target = lerp(P2, P1, 1-bias); // lerp f
      if (i==0) setpointattrib(0,"P", first, target );
      if (i==numdiv) setpointattrib(0,"P", second, target );
      int newPt = addpoint(0,target);
      setpointgroup(0,"edge",newPt,1);
      int new_prim0 = generate_poly ( base_pt,newPt,pt_current,primnum);
      pt_current = newPt;
      setprimgroup(0,"subdivide",new_prim0,1);
      removeprim(0,primnum,1);
    }
  return 1;
}
//
vector average_incident (int ptnum; int classes[]){
  int classes_ordered[] = classes;
  int first =  classes_ordered[0];
  int second = classes_ordered[1];
  int edges1[] = {};
  int edges2[] = {};
  int primary_hedges[]={};
  int hout = pointhedge(0, ptnum);
  while ( hout != -1 )
    {
      if (hedge_isprimary(0, hout)) push ( primary_hedges,hout );
      int hin = hedge_prev(0, hout);
      if (hedge_isprimary(0, hin)) push ( primary_hedges,hin);
      hout = pointhedgenext(0, hout);
    }
  foreach (int hedge; primary_hedges){
    int pr = hedge_prim(0, hedge) ;
    int its_class = prim(0,"class",pr);
    int its_equiv_class = get_hedge_adj_prim_class (hedge);
    if ( its_class == its_equiv_class ){
      if (its_class == first) push(edges1, hedge);
      else push(edges2, hedge);
    }
  }
  vector average_first,average_second  = set(0,0,0);
  foreach (int hedge; edges1){
    int src_pt = hedge_srcpoint(0,hedge );
    int dst_pt = hedge_dstpoint(0,hedge );
    vector P1 = point(0,"P",src_pt );
    vector P2 = point(0,"P",dst_pt );
    average_first += (P2-P1);
  }
  average_first/=len( average_first );
  foreach (int hedge; edges2){
    int src_pt = hedge_srcpoint(0,hedge );
    int dst_pt = hedge_dstpoint(0,hedge );
    vector P1 = point(0,"P",src_pt );
    vector P2 = point(0,"P",dst_pt );
    average_second += (P1-P2);
  }
  average_second/=len( average_second );
  vector averageV=((average_second)+(average_first))/2;
  return averageV;

}

int split_in_half(int primnum; int sharebase_prim){

  int apex,apex2=-1;
  int points[]={};
  int prim_num=primnum;
  // find the share base prim apex
    int points2[]={};
    int my_points[]=primpoints(0,primnum);
    int counter_pts[] = primpoints(0,sharebase_prim );
    foreach( int pt; counter_pts ) {
      if (in_the (my_points, pt)) append(points2,pt);
    }
    foreach( int pt; counter_pts ) {
      if (pt !=points2[1] && pt !=points2[0]  ) apex2 =pt;
    }
    foreach( int pt; my_points ) {
      if (pt !=points2[1] && pt !=points2[0]  ) apex=pt;
    }
    vector p1 = point(0,"P",points2[0]);
    vector p2 = point(0,"P",points2[1]);
    vector averageP = (p1+p2)/2;
    int basept = addpoint(0,averageP);
    // add and set up attrs
    int new_prim1 = generate_poly ( basept, apex, points2[0],primnum);
    int new_prim2 = generate_poly ( points2[1], apex, basept,primnum);
    int new_prim3 = generate_poly ( apex2, basept, points2[0],sharebase_prim);
    int new_prim4 = generate_poly ( points2[1], apex2, basept,sharebase_prim);

    int new_prims[] = array (new_prim1 ,new_prim2 , new_prim3, new_prim4 );
    foreach(int pr; new_prims) {setprimgroup (0, "split", pr,1);}

    return basept;
}

int build_strings(int iter;int seeds[] ;string geo_pairs[];string pairs_to_build[]){
   // will return number of forks
  int c=0;
  int newbies=0;
  int old_n=0;
  for(int i =0; i<iter;i++){
    int new_seeds[] = {};
    foreach (int seed; seeds){
      foreach (string pair; geo_pairs){
        string b[] = split(pair,"_");
        int first = atoi(b[0]);
        int second = atoi(b[1]);
        if (first == seed) {
          append_if_not_in(pairs_to_build,pair);
          append_if_not_in(new_seeds,second);
          ;}
        if (second == seed) {
          append_if_not_in(pairs_to_build,pair);
          append_if_not_in(new_seeds,first);
          ;}
        if( second == seed || first == seed ) c+=1;
      }
      seeds = new_seeds;
                             }
    newbies = c-old_n;
    old_n = c;

  }
  return newbies;
}

void build_pairs_from_primlist(string sliced_pairs_list[]){
  int idx = 0;
  foreach (string pair; sliced_pairs_list){
    string b[] = split( pair ,"_");
    int first = atoi(b[0]);
    int second = atoi(b[1]);
    vector p1 = point(0, "P", first);
    vector p2 = point(0, "P", second);
    int pt1 = addpoint(0, p1);
    int pt2 = addpoint(0, p2);
    int pr = addprim(0, "polyline", pt1, pt2);
    setprimgroup(0, "border", pr, 1);
    setprimattrib( 0, "time", pr, idx  );
    idx++;
  } }
#endif
