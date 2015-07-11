open Dba
open Formula
open Concsymb_policy
open CommonAnalysis
open Trace_type
open TraceAnalysis
open AnalysisEnv
open Formula
open Smtlib2
open Solver
open Smtlib2print
open Basic_structs
open Concsymb_policy
open Random
open Extlib

type examination_t =
    Visited          (* visited *)
  | Covered          (* visited and completely covered *)
  | PartiallyCovered (* visited but partially covered only *)
  | JustCovered      (* visited, covered by solving the predicate at some previous instruction (for dynamic jump) *)
  | Uncoverable      (* visited, tried to cover, but cannot *)

type state_indentifier_t = Memory of int | Register of string

type continuation_t =
  {
    next_location : int64;
    input_value   : int list;
  }

type control_t = ConJump | DynJump

type exe_control_point_t =
  {
    location         : int64;
    history          : int64 list;
    continuations    : continuation_t DynArray.t;
    mutable explored : examination_t;
    control_type     : control_t
  }

 exception NotVisitedContinuationIndex of int
 exception AllContinuationVisited

class exp_policy_a = object(self)
  (* method concsymb_store (tr_index:int) (addr_occ:int) (e_addr:dbaExpr) (e:dbaExpr) (size:int) (inst:trace_inst): concsymb_action * concsymb_action = *)
  method concsymb_store (tr_index:int) (addr_occ:int) (e_addr:dbaExpr) (e:dbaExpr) (size:int) (nth_dba:int) (inst:trace_inst): concsymb_action * concsymb_action =
    if (tr_index = 1)                           (* push ebp *)
    then (KeepOrConc(false), KeepOrConc(false)) (* concretize both stored address and stored value *)
    else if (tr_index = 3)                      (* push esi *)
    then (KeepOrConc(false), KeepOrConc(false)) (* concretize both stored address and stored value *)
    else if (tr_index = 4)                      (* push ebx *)
    then (KeepOrConc(false), KeepOrConc(false)) (* concretize both stored address and stored value *)
    else if (tr_index = 7)                      (* mov [ebp-0xa8],eax *)
    then (KeepOrConc(false), KeepOrConc(false)) (* concretize both stored address and stored value *)
    else if (tr_index = 8)                      (* mov [ebp-0xac],0x804b100 <- the address of jump table *)
    then (KeepOrConc(false), KeepOrConc(false)) (* concretize both stored address and stored value *)
    else (KeepOrConc(true), KeepOrConc(true))   (* keep the rest as-is *)

  method concsymb_load (tr_index:int) (add_occ:int) (e:dbaExpr) (size:int) (nth_dba:int) (inst:trace_inst): concsymb_action * concsymb_action =
    (KeepOrSymb(true), KeepOrSymb(true))        (* keep as-is *)

  (* method concsymb_var (tr_index:int) (addr_occ:int) (name:string) (low:int) (hight:int) (nth_dba:int) (inst:trace_inst): concsymb_action = *)
  (*   if (tr_index = 2)                           (\* mov ebp,esp *\) *)
  (*   then (KeepOrConc(false), KeepOrConc(false)) (\* concretize assigned value *\) *)
  (*   else (KeepOrConc(true), KeepOrConc(true))   (\* keep the rest as-is *\) *)
end;;

(* ================================================================================ *)

class exp_policy_b = object(self)

  method concsymb_store (tr_index:int) (addr_occ:int) (e_addr:dbaExpr) (e:dbaExpr) (size:int) (nth_dba:int) (inst:trace_inst): concsymb_action * concsymb_action =
    (* if (Int64.compare (Int64.of_string "0x08048c22") inst.location) = 0 *)
    (* then (Conc, KeepOrSymb(false)) *)
    (* (\* else if (Int64.compare (Int64.of_string "0x08048c4b") inst.location) = 0 *\) *)
    (* (\* then (KeepOrConc(true), Conc) *\) *)
    (* (\* else if (Int64.compare (Int64.of_string "0x08048c43") inst.location) <= 0 *\) *)
    (* (\* then (Conc, KeepOrSymb(true)) *\) *)
    (* else (KeepOrSymb(false), KeepOrSymb(false)) *)

    (* if (tr_index <= 3) then (Conc, KeepOrSymb(false))
       else (KeepOrSymb(false), KeepOrSymb(false)) *)

    (KeepOrSymb(false), KeepOrSymb(false))


  method concsymb_load (tr_index:int) (add_occ:int) (e:dbaExpr) (size:int) (nth_dba:int) (inst:trace_inst) =
    (KeepOrSymb(false), KeepOrSymb(false))

  method concsymb_var (tr_index:int) (addr_occ:int) (name:string) (low:int) (hight:int) (nth_dba:int) (inst:trace_inst) =
    if (name = "ebp") || (name = "esp") (* make concrete action records of all functions *)
    then Conc
    else KeepOrSymb(false)

  method intended_concsymb (tr_index:int) (addr_occ:int) (inst:trace_inst) (nth_dba:int) (env:analysis_env): unit = ()
end;;

(* ============================================================================= *)

let construct_memory_state (base_address:int) (state_entries:int list) initial_state =
  let
    accum_state (state, addr_offset) state_entry =
    let next_state = Addr64Map.add (Int64.add (Int64.of_int base_address) addr_offset) state_entry state
    and next_addr_offset = Int64.add addr_offset 1L
    in (next_state, next_addr_offset)
  in
  (* fold_left : ('a -> 'b -> 'a) -> 'a -> 'b list -> 'a *)
  fst (List.fold_left accum_state (initial_state, 0L) state_entries)

(* ============================================================================= *)

let construct_memory_state_from_file base_address state_entries_filename initial_state =
  let entries_as_string = Std.input_file ~bin:false state_entries_filename in
  let entries_as_strings = Str.split (Str.regexp "[;]") entries_as_string in
  (
    (* Printf.printf "read %d entries from dump file %s\n" (List.length entries_as_strings) state_entries_filename; *)
    (* List.iter (fun entry -> *)
    (*     let value = (int_of_string entry) in Printf.printf "%d " value) entries_as_strings; *)

    let entries = List.map (fun str -> int_of_string str) entries_as_strings in
    construct_memory_state base_address entries initial_state
  )

(* ============================================================================= *)

let find_not_visited_continuation_index control_point =
  try
    let idx = ref 0 in
    DynArray.iter (fun continuation ->
        if (Int64.to_int continuation.next_location = 0) then raise (NotVisitedContinuationIndex !idx)
        else idx := !idx + 1) control_point.continuations;
    None
  with
  | NotVisitedContinuationIndex i -> Some i

(* ============================================================================= *)

let find_last_visited_continuation_index control_point =
  try
    let idx = ref 0 in
    DynArray.iter (fun continuation ->
        if (Int64.to_int continuation.next_location <> 0)
        then idx := !idx + 1
        else raise (NotVisitedContinuationIndex !idx)) control_point.continuations;
      Some (!idx - 1)
  with
  | NotVisitedContinuationIndex i -> Some i

(* ============================================================================= *)

let find_visiting_continuation_index control_point =
  try
    let idx = ref 0 in
    DynArray.iter (fun continuation ->
      if (Int64.to_int continuation.next_location <> 0) then idx := !idx + 1
      else raise (NotVisitedContinuationIndex !idx)) control_point.continuations;
    Some (!idx - 1)
  with
  | NotVisitedContinuationIndex i -> if (i = 0) then Some i else Some (i - 1)

(* ============================================================================= *)

(* moteur d'execution spécial *)
class explorer_b (trace_filename:string) concolic_policy = object(self) inherit trace_analysis trace_filename concolic_policy as super
  val jump_table_address = 0x804a01c
  val jump_table_dump_file = "mobfus.dump"

  val mutable input_is_parameterized = false
  val mutable start_exploring = false

  (* the control point (conditional or dynamic jumps) that we need to explore *)
  val mutable target_control_point =
    {
      location        = Int64.of_int 0;
      history         = [];
      continuations   = DynArray.create ();
      explored        = Visited;
      control_type    = ConJump
    }

  method set_target_control_point cpoint = (target_control_point <- cpoint)
  method get_target_control_point = target_control_point

  (* in executing to the target control point, we need re-tracing the program, the following list respresent the current trace *)
  val accumulated_ins_locs : (int64 DynArray.t) = DynArray.create ()
  val mutable current_inst_idx = 0

  (* the new control points that we will visit by following a new direction of the target control point *)
  val new_visited_control_points : (exe_control_point_t DynArray.t) = DynArray.create ()
  method get_new_visited_control_points = new_visited_control_points

  (* the control points that we have visited *)
  val visited_control_points : (exe_control_point_t DynArray.t) = DynArray.create ()
  method set_visited_control_points cpoints =
    (DynArray.clear visited_control_points);
    (DynArray.append cpoints visited_control_points)

  (* input points *)
  (* val input_points = [ (0x8048cc5,0xffffd0d7); (0x8048cce,0xffffd0d6); (0x8048cd7,0xffffd0d5); (0x8048ce0,0xffffd0d4); *)
  (*                      (0x8048ce9,0xffffd0d3); (0x8048cf2,0xffffd0d2); (0x8048cfb,0xffffd0d1)] *)
  val input_points = [ (0x08048429, Register "eax") ]
  method get_input_points = input_points
  val mutable input_vars = []
  method get_input_vars = input_vars


  method private get_dynamic_jmp_new_input_values target_expr target_var_name input_var_names current_target_addr_input_values_pair init_mem_state env =
    (self#add_witness_variable target_var_name target_expr env);
    let formula_file = "formula_djump.smt2" in
    let rec get_possible_targets collected_target_addr_input_values_pairs =
      let collected_addrs = fst (List.split collected_target_addr_input_values_pairs) in
      (* fold_left : ('a -> 'b -> 'a) -> 'a -> 'b list -> 'a *)
      let local_pred = List.fold_left (fun current_pred addr ->
          (SmtNot(self#build_witness_bitvector_comparison_predicate target_var_name env.addr_size addr))::current_pred)
          [] collected_addrs
      in
      let trace_pred = self#build_multiple_condition_predicate local_pred in
      (
        (build_formula env.formula trace_pred ~negate:false ~initial_state:init_mem_state formula_file);
        let result, model = solve_z3_model formula_file in
        match result with
        | SAT ->
          (
            let input_values = List.map (fun input_var_name -> Big_int.int_of_big_int (fst (get_bitvector_value model input_var_name)))  input_var_names
            and target_addr = get_bitvector_value model target_var_name in
            get_possible_targets ((target_addr, input_values)::collected_target_addr_input_values_pairs)
          )
        | UNSAT -> collected_target_addr_input_values_pairs
      )
    in
    (
      (* copy from http://langref.org/fantom+ocaml+java/lists/modification/remove-last *)
      let remove_last lls =
        match (List.rev lls) with
        | h::t -> List.rev t
        | [] -> []
      in
      let all_targets = get_possible_targets [current_target_addr_input_values_pair] in
      snd (List.split (remove_last all_targets))
    )


  method private get_conditional_jmp_new_input_values target_cond input_var_names current_cond_prop init_mem_state env =
    let formula_file = "formula_if.smt2"
    and trace_pred = self#build_cond_predicate target_cond env in
    (
      (build_formula env.formula trace_pred ~negate:current_cond_prop ~initial_state:init_mem_state formula_file);
      let result, model = solve_z3_model formula_file in
      (
        match result with
        | SAT -> List.map (fun input_var_name -> Big_int.int_of_big_int (fst (get_bitvector_value model input_var_name))) input_var_names
        | UNSAT -> []
      )
    )


  (* ================================== visiting methods ================================== *)

  method visit_instr_before (key:int) (inst:trace_inst) (env:analysis_env) = (* of type trace_visit_action *)
    DoExec

  method visit_instr_after (key:int) (inst:trace_inst) (env:analysis_env) =
    (current_inst_idx <- current_inst_idx + 1);
    (DynArray.add accumulated_ins_locs inst.location);
    DoExec


  method private add_new_control_point inst dbainst addr_size =
    let current_conti_idx = find_visiting_continuation_index target_control_point in
    match current_conti_idx with
    | Some idx ->
      (
        let current_conti = DynArray.get target_control_point.continuations idx in
        let current_input_value = current_conti.input_value in
        let new_continuation = { next_location = Big_int.int64_of_big_int (fst (get_next_address inst.concrete_infos addr_size));
                                 input_value = current_input_value } in
        let c_type =
          match snd dbainst with
          | DbaIkIf _ -> ConJump
          | DbaIkDJump _ -> DynJump
          | _ -> assert false
        in
        let new_visited_control_point = { location      = inst.location;
                                          history       = DynArray.to_list accumulated_ins_locs;
                                          continuations = DynArray.init 1 (fun _ -> new_continuation);
                                          explored      = Visited;
                                          control_type  = c_type }
        in
        (
          Printf.printf "add new control point at 0x%x with history %d\n" (Int64.to_int new_visited_control_point.location)
            (List.length new_visited_control_point.history);

          (DynArray.add new_visited_control_points new_visited_control_point);
        )
      )
    | _ -> assert false


  method private update_dynamic_jump_continuations dbainst inst (env:analysis_env) =
    match snd dbainst with
    | DbaIkAssign (DbaLhsVar(var, size, tags), expr, offset) ->
      (
        if self#is_symbolic_expression expr env
        then
          (
            (* Printf.printf "in: we are here\n"; flush stdout; *)
            (* let init_state = construct_memory_state jump_table_address jump_table_entries Addr64Map.empty *)
            let init_state = construct_memory_state_from_file jump_table_address jump_table_dump_file Addr64Map.empty
            and current_target_addr = get_regwrite_value_bv "ecx" inst.concrete_infos env.addr_size
            and current_input_value = (DynArray.get target_control_point.continuations 0).input_value
            in
            let new_input_values = self#get_dynamic_jmp_new_input_values
                expr "jump_address" input_vars (current_target_addr, current_input_value) init_state env
            in
            (
              match new_input_values with
              | [] ->
                (
                  (target_control_point.explored <- Uncoverable);

                  Printf.printf "dynamic jump at 0x%x with history %d is UNCOVERABLE\n" (Int64.to_int target_control_point.location)
                    (List.length target_control_point.history); flush stdout;
                )
              | input_values ->
                (
                  (target_control_point.explored <- JustCovered);

                  Printf.printf "dynamic jump at 0x%x with history %d is JUST COVERED\n" (Int64.to_int target_control_point.location)
                    (List.length target_control_point.history); flush stdout;

                  List.iter (fun value ->
                      let new_continuation = { next_location = Int64.of_int 0; input_value = value }
                      in DynArray.add target_control_point.continuations new_continuation
                    ) input_values
                )
            );
            (* Printf.printf "out: we are here\n"; flush stdout; *)
          )
        else
          (
            (target_control_point.explored <- Uncoverable);

            Printf.printf "dynamic jump at 0x%x with history %d is UNCOVERABLE.\n" (Int64.to_int target_control_point.location)
              (List.length target_control_point.history); flush stdout;
          )
      )
    | _ -> ()


  method private update_dynamic_jump_next_continuation inst (env:analysis_env) =
    match find_not_visited_continuation_index target_control_point with
    | Some idx ->
      (
        let new_continuation = { (DynArray.get target_control_point.continuations idx)
                                 with next_location = Big_int.int64_of_big_int (fst (get_next_address inst.concrete_infos env.addr_size))}
        in DynArray.set target_control_point.continuations idx new_continuation;

        if (idx = DynArray.length target_control_point.continuations - 1)
        then
          (
            (target_control_point.explored <- Covered);

            Printf.printf "dynamic jump at 0x%x with history %d is COMPLETELY COVERED\n" (Int64.to_int target_control_point.location)
              (List.length target_control_point.history); flush stdout;
          )
        else
          (
            (target_control_point.explored <- PartiallyCovered);

            Printf.printf "dynamic jump at 0x%x with history %d is PARTIALLY COVERED\n" (Int64.to_int target_control_point.location)
              (List.length target_control_point.history); flush stdout;
          )
        (* DoExec *)
      )
    | None -> assert false;


  method private update_dynamic_jump_first_continuation inst (env:analysis_env) =
    let new_continuation = { (DynArray.get target_control_point.continuations 0)
                             with next_location = Big_int.int64_of_big_int (fst (get_next_address inst.concrete_infos env.addr_size)) }
    in DynArray.set target_control_point.continuations 0 new_continuation;

    Printf.printf "dynamic jump at 0x%x with history %d is PARTIALLY COVERED\n" (Int64.to_int target_control_point.location)
      (List.length target_control_point.history); flush stdout;

    (target_control_point.explored <- PartiallyCovered);


  method private calculate_conditional_jump_continuations cond address inst (env:analysis_env) =
    let cond_prop = Big_int.eq_big_int address (fst (get_next_address inst.concrete_infos env.addr_size))
    (* and init_state = construct_memory_state new_jump_table_address new_jump_table_entries Addr64Map.empty in *)
    and init_state = construct_memory_state_from_file jump_table_address jump_table_dump_file Addr64Map.empty in
    let new_input_values = self#get_conditional_jmp_new_input_values cond input_vars cond_prop init_state env in
    (
      match new_input_values with
      | [] ->
        (
          (target_control_point.explored <- Uncoverable);

          Printf.printf "conditional jump at 0x%x with history %d is UNCOVERABLE\n" (Int64.to_int target_control_point.location)
            (List.length target_control_point.history); flush stdout;
        )
      | values ->
        (
          let new_continuation = { next_location = Int64.of_int 0; input_value =  values }
          in DynArray.add target_control_point.continuations new_continuation;

          (target_control_point.explored <- PartiallyCovered);

          Printf.printf "conditional jump at 0x%x with history %d is PARTIALLY COVERED\n" (Int64.to_int target_control_point.location)
            (List.length target_control_point.history); flush stdout;
        )
    );


  method private update_conditional_jump_next_continuation inst (env:analysis_env) =
    let new_continuation = { (DynArray.get target_control_point.continuations 1)
                             with next_location = Big_int.int64_of_big_int (fst (get_next_address inst.concrete_infos env.addr_size)) }
    in DynArray.set target_control_point.continuations 1 new_continuation;

    (target_control_point.explored <- Covered);

    Printf.printf "conditional jump at 0x%x with history %d is COMPLETELY COVERED\n" (Int64.to_int target_control_point.location)
      (List.length target_control_point.history); flush stdout;


  method visit_dbainstr_before (key:int) (inst:trace_inst) (dbainst:dbainstr) (env:analysis_env) =
    if (List.length target_control_point.history = 0) || (DynArray.length accumulated_ins_locs > List.length target_control_point.history)
    then
      (
        (* Printf.printf "accumulated instruction number %d\n" (DynArray.length accumulated_ins_locs); flush stdout; *)
        match target_control_point.explored with
        | Visited | Covered | PartiallyCovered ->
          (
            match snd dbainst with
            | DbaIkIf (_, NonLocal((_, _), _), _) | DbaIkDJump _ ->
              self#add_new_control_point inst dbainst env.addr_size
            | _ -> ()
          );
          DoExec
        | _ -> assert false
      )
    else
      (
        if (DynArray.length accumulated_ins_locs = List.length target_control_point.history)
        then
          (
            match snd dbainst with
            | DbaIkIf (cond, NonLocal((address, _), _), offset) ->
              if self#is_symbolic_condition cond env
              then
                (
                  (* Printf.printf "control point with history %d may be coverable\n" (List.length target_control_point.history); flush stdout; *)
                  match target_control_point.explored with
                  | PartiallyCovered ->
                    (
                      (self#update_conditional_jump_next_continuation inst env);
                      DoExec
                    )
                  | Visited ->
                    (
                      (self#calculate_conditional_jump_continuations cond address inst env);
                      StopExec
                    )
                  | _ ->
                    (
                      (* Printf.printf "Failed at 0x%x %s\n" (Int64.to_int inst.location) inst.opcode; flush stdout; *)
                      assert false
                      (* DoExec *)
                    )
                )
              else
                (
                  Printf.printf "conditional jump at 0x%x with history %d is UNCOVERABLE\n" (Int64.to_int target_control_point.location)
                    (List.length target_control_point.history); flush stdout;

                  (target_control_point.explored <- Uncoverable);
                  StopExec
                )
            | DbaIkDJump _ ->
              (
                match target_control_point.explored with
                | JustCovered ->
                  (
                    (self#update_dynamic_jump_first_continuation inst env);
                    StopExec
                  )
                | PartiallyCovered ->
                  (
                    (self#update_dynamic_jump_next_continuation inst env);
                    DoExec
                  )
                | Uncoverable -> StopExec
                | _ ->
                  (
                    Printf.printf "Failed at 0x%x %s\n" (Int64.to_int inst.location) inst.opcode;
                    assert false
                  )
              )

            | _ ->
              (
                (* remember that an instruction contains several dba instructions then the dba
                   instrution handled here may not be a conditional nor a dynamic jump *)
                DoExec
              )
          )
        else
          (
            if (DynArray.length accumulated_ins_locs = List.length target_control_point.history - 1)
            then
              match target_control_point.control_type with
              | DynJump ->
                (
                  match target_control_point.explored with
                  | Visited -> self#update_dynamic_jump_continuations dbainst inst env
                  | _ -> ()
                )
              | _ -> ()
            else ();
            DoExec
          )
      )

  (* ============================================================================= *)
  (* mark the input *)
  method visit_dbainstr_after (key:int) (inst:trace_inst) (dbainst:dbainstr) (env:analysis_env) =
    let add_var_char_constraints_into_env var_name upper_bound lower_bound =
      let lt = SmtBvBinary(SmtBvSlt,
                           SmtBvVar(var_name, 32),
                           SmtBvCst(Big_int.big_int_of_int upper_bound, 32))
      and ge = SmtBvBinary(SmtBvSge,
                           SmtBvVar(var_name, 32),
                           SmtBvCst(Big_int.big_int_of_int lower_bound, 32))
      in
      (
        env.formula <- add_constraint env.formula ~comment:(Some "upper bound constraint") (SmtBvExpr(lt));
        env.formula <- add_constraint env.formula ~comment:(Some "lower bound constraint") (SmtBvExpr(ge));
      )
    in
    (* let ins_locs = fst (List.unzip input_points) in *)
    (* let ins_locs = (List.map (fun input_point -> fst input_point) input_points) in *)
    let ins_locs = fst (List.split input_points) in
    if (List.exists (fun loc -> Int64.compare (Int64.of_int loc) inst.location = 0) ins_locs)
    then
      (
        match (snd dbainst) with
        | DbaIkAssign(lhs, expr, offset) ->
          (
            let var_name = "x_"^(Printf.sprintf "0x%x" (Int64.to_int inst.location)) in
            (
              (* Printf.printf "add input variable %s\n" var_name; flush stdout; *)
              self#add_witness_variable var_name expr env;
              add_var_char_constraints_into_env var_name 127 0;
              input_vars <- input_vars@[var_name]
            )
          )
        | _ -> ()
      )
    else ();
    DoExec
end;;

(* ============================================================================= *)

let generate_option_file exe_filename start_addr stop_addr =
  let option_info = Printf.sprintf "start,0x%x\nstop,0x%x" start_addr stop_addr
  and option_filename = (Filename.basename exe_filename) ^ ".opt"
  in
  (
    (Std.output_file option_filename option_info);
    ignore (Printf.printf "option file %s\n", option_filename);
    option_filename
  )

(* ============================================================================= *)

let generate_config_file exe_filename ins_address before_or_after reg_name reg_value =
  let config_info = (Printf.sprintf "0x%x" ins_address) ^ "," ^ (* location of instruction *)
                    "1," ^                                      (* execution order (always 1 if instruction is outside any loop) *)
                    (Printf.sprintf "%s:0:31" reg_name) ^ "," ^ (* register name *)
                    (Printf.sprintf "0x%x" reg_value) ^ "," ^   (* register value *)
                    (Printf.sprintf "%d" before_or_after)       (* patching point (0 = before, 1 = after) *)
  and config_filename = (Filename.basename exe_filename) ^ ".conf"
  in
  (
    (Std.output_file config_filename config_info);
    config_filename
  )

(* ============================================================================= *)

let generate_config_file exe_filename inputs =
  let config_of_input =
    match inputs with
    | ((ins_addr, Memory mem_addr), mem_value)::_ ->
      fun ((ins_addr, Memory mem_addr), mem_value) -> (Printf.sprintf "0x%x" ins_addr) ^ "," ^      (* location of instruction *)
                                                      "1," ^                                        (* execution order (always 1 if instruction is outside any loop) *)
                                                      (Printf.sprintf "0x%x:%d" mem_addr 1) ^ "," ^ (* memory address *)
                                                      (Printf.sprintf "0x%x" mem_value) ^ "," ^     (* memory value  *)
                                                      "1\n"                                         (* patching point (0 = before, 1 = after) *)
    | ((ins_addr, Register reg_name), reg_value)::_ ->
      fun ((ins_addr, Register reg_name), reg_value) -> (Printf.sprintf "0x%x" ins_addr) ^ "," ^    (* location of instruction *)
                                                        "1," ^                                      (* execution order (always 1 if instruction is outside any loop) *)
                                                        (Printf.sprintf "%s:0:31" reg_name) ^ "," ^ (* register name *)
                                                        (Printf.sprintf "0x%x" reg_value) ^ "," ^   (* register value *)
                                                        "1\n"                                       (* patching point (0 = before, 1 = after) *)
    | _ ->
      (
        Printf.printf "malformed input information\n"; flush stdout;
        assert false
      )
  in
  let config_info = List.fold_left (fun acc_conf input -> acc_conf ^ (config_of_input input)) "" inputs
  and config_filename = (Filename.basename exe_filename) ^ ".conf"
  in
  (
    (* Printf.printf "generate new configuration file\n"; flush stdout; *)
    (Std.output_file config_filename config_info);
    config_filename
  )

(* ============================================================================= *)

let instrument_exe exe_filename option_filename config_filename =
  let trace_filename = (Filename.basename exe_filename ^ ".trace") in
  let instrument_cmd = "./pin67257/ia32/bin/pinbin -t ./pintools/trace-pin/build/vtrace.pin" ^ " -opt " ^ option_filename ^
                       " -conf " ^ config_filename ^ " -out " ^ trace_filename ^ " -- " ^ exe_filename ^ " 57" in
  (
    let inout_channels = Unix.open_process instrument_cmd in
    let exit_status    = Unix.close_process inout_channels in
    match exit_status with
    | Unix.WEXITED exit_code -> Some trace_filename
    | _ -> None
  )

(* ============================================================================= *)

(* exploration strategy *)
let find_next_unexplored_control_point visited_control_points =
  try
    Some
      (
        List.find(fun cpoint -> match cpoint.explored with
            | Visited | PartiallyCovered -> true
            | _ -> false)
          (DynArray.to_list visited_control_points)
      )
  with
  | Not_found -> None

(* ============================================================================= *)

let get_exploration_input control_point =
  match control_point.explored with
  | Visited -> (DynArray.get control_point.continuations 0).input_value
  | PartiallyCovered ->
    (
      let cont_idx = find_not_visited_continuation_index control_point in
      match (cont_idx) with
      | Some idx -> (DynArray.get control_point.continuations idx).input_value
      | None -> assert false
    )
  | _ -> assert false

(* ============================================================================= *)

let find_control_point_index cpoint cpoints =
  let i = ref 0
  and found = ref false
  and cpoints_length = (DynArray.length cpoints) in
  (
    while ((not !found) && (!i < cpoints_length)) do
      if ((DynArray.get cpoints !i).location = cpoint.location) &&
         ((List.length (DynArray.get cpoints !i).history) = (List.length cpoint.history))
      then found := true
      else i := !i + 1
    done;
    if !found then Some !i else None
  )

(* ============================================================================= *)

let create_pseudo_control_point () =
  Random.self_init ();
  let i = ref 0
  and random_inputs = ref [] in
  (
    while !i < 1 do
      random_inputs := (Random.int 127)::!random_inputs;
      i := !i + 1
    done;

    let pseudo_continuation =
      {
        next_location = Int64.of_int 0;
        input_value   = !random_inputs
      }
    in
    (
      Printf.printf "initial input values: ";
      List.iter (fun input -> ignore (Printf.printf "0x%x " input)) !random_inputs;
      Printf.printf "(randomized)\n"; flush stdout;
      Some (* create a pseudo control point for the first time *)
        {
          location      = Int64.of_int 0;
          history       = [];
          continuations = DynArray.init 2 (fun _ -> pseudo_continuation);
          explored      = Visited;
          control_type  = ConJump
        };
    )
  )

(* ============================================================================= *)

let get_exploration_control_point visited_cpoints =
  if (DynArray.empty visited_cpoints)
  then create_pseudo_control_point ()
  else find_next_unexplored_control_point visited_cpoints

(* ============================================================================= *)

let print_exploration_result visited_cpoints =
  Printf.printf "===================================================\nexploration results:\n";
  DynArray.iter (fun cpoint ->
      match cpoint.explored with
      | Covered ->
        (
          (
            match cpoint.control_type with
            | ConJump -> Printf.printf "conditional jump at 0x%x with history %d is covered by:\n" (Int64.to_int cpoint.location) (List.length cpoint.history)
            | DynJump -> Printf.printf "dynamic jump at 0x%x with history %d is covered by:\n" (Int64.to_int cpoint.location) (List.length cpoint.history)
          );

          DynArray.iter (fun continuation ->
              Printf.printf "next address: 0x%x; " (Int64.to_int continuation.next_location);
              Printf.printf "input value(s): ";
              List.iter (fun value -> Printf.printf "0x%x " value) continuation.input_value;
              Printf.printf "\n"
            ) cpoint.continuations
        )
      | _ -> ()
    ) visited_cpoints


(* ============================================================================= *)
(* trace explorer for conditional and dynamic jumps *)
let explore_exe (exe_filename:string) (start_addr:int) (stop_addr:int) =
  let visited_cpoints = DynArray.create ()
  and all_explored    = ref false
  and option_filename = generate_option_file exe_filename start_addr stop_addr
  and cs_policy       = new exp_policy_b
  in
  while not !all_explored do
    let next_cpoint = get_exploration_control_point visited_cpoints in
    match next_cpoint with
    | None ->
      (
        all_explored := true;
        Printf.printf "all control points are covered, stop exploration.\n";
        print_exploration_result visited_cpoints;
        flush stdout;
      )
    | Some cpoint ->
      (
        let config_filename =
          let input_points = [ (0x08048429, Register "eax") ]
          and input_values = get_exploration_input cpoint
          in
          (
            if Int64.to_int cpoint.location = 0
            then Printf.printf "VISIT pseudo control point with input value(s): "
            else Printf.printf "REVISIT control point at 0x%x with input value(s): " (Int64.to_int cpoint.location);
            List.iter (fun input -> ignore (Printf.printf "0x%x " input)) input_values;
            Printf.printf "\n"; flush stdout;

            generate_config_file exe_filename (List.combine input_points input_values)
          )
        in
        (
          match instrument_exe exe_filename option_filename config_filename with
          | None -> exit 0
          | Some trace_filename ->
            let exp_instance = new explorer_b trace_filename cs_policy in
            (
              (exp_instance#set_visited_control_points visited_cpoints);
              (exp_instance#set_target_control_point cpoint);
              (exp_instance#compute);

              if (DynArray.empty visited_cpoints)
              then ()
              else
                (
                  let current_target_cpoint = exp_instance#get_target_control_point in
                  match find_control_point_index current_target_cpoint visited_cpoints with
                  | None ->
                    (
                      Printf.printf "instruction index not found\n"; flush stdout;
                      assert false
                    )
                  | Some idx ->
                    (
                      DynArray.set visited_cpoints idx current_target_cpoint
                    )
                );
                DynArray.append exp_instance#get_new_visited_control_points visited_cpoints
            )
        )
      )
  done
