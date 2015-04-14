(* catamorphism *)
type ('a, 'b) list_cata_t = 'b * ('a -> 'b -> 'b)

(* let list_cata (a, f) = *)
(*   let rec cata = function *)
(*     | [] -> a *)
(*     | (x::l) -> f x (cata l) *)
(*   in cata *)

let list_cata : ('a, 'b) list_cata_t -> 'a list -> 'b = fun (a, f) ->
  let rec cata = function
    | [] -> a
    | (x::l) -> f x (cata l)
  in cata

let prod = list_cata (1, ( * ))

(* anamorphism *)
type ('a, 'b) list_ana_t = 'a -> ('b * 'a) option

(* let list_ana  a = *)
(*   let rec ana u = match a u with *)
(*     | None -> [] *)
(*     | Some (x, l) -> x :: (ana l) *)
(*   in ana *)

let list_ana : ('a, 'b) list_ana_t -> 'a -> 'b list = fun a ->
(* let list_ana = fun a -> *)
  let rec ana u = match a u with
    | None -> []
    | Some (x, l) -> x :: (ana l)
    (* | _ -> [] *)
  in ana

let count =
  let destruct_count = function
    | 0 -> None
    | n -> Some (n, n - 1)
  in list_ana destruct_count

let product_from_count = fun n -> prod (count n)

(* unit type pattern matching for entry point *)
let () =
  if (Array.length Sys.argv != 2) then
    Printf.printf "invalid input\n"
  else
    Printf.printf "%d " (product_from_count (Pervasives.int_of_string Sys.argv.(1)))
    (* (\* Printf.printf "[%s]\n" Sys.argv.(0) *\) *)
    (* let list_from_ana = count (Pervasives.int_of_string Sys.argv.(1)) in *)
    (* let product_of_list = prod list_from_ana in *)
    (* Printf.printf "%d " product_of_list *)
    (* (\* in List.iter (Printf.printf "%d ") list_from_ana *\) *)

(* let _ = main *)
