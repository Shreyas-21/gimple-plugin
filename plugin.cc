#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

#include "gcc-plugin.h"
#include "plugin-version.h"

#include "basic-block.h"
#include "context.h"
#include "cp/cp-tree.h"
#include "gimple-expr.h"
#include "gimple-pretty-print.h"
#include "gimple.h"

#include "gimple-iterator.h"
#include "print-tree.h"
#include "stringpool.h"
#include "tree-core.h"
#include "tree-pass.h"
#include "tree.h"
// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = {
    "1.0", "This plugin suggests using const function arguments for C"};

const pass_data warn_add_const_data = {
    GIMPLE_PASS,
    "warn_const_argument_cxx", /* name */
    OPTGROUP_NONE,            /* optinfo_flags */
    TV_NONE,                  /* tv_id */
    PROP_gimple_any,          /* properties_required */
    0,                        /* properties_provided */
    0,                        /* properties_destroyed */
    0,                        /* todo_flags_start */
    0                         /* todo_flags_finish */
};

struct warn_add_const : gimple_opt_pass {
  warn_add_const(gcc::context *ctx)
      : gimple_opt_pass(warn_add_const_data, ctx) {}

  virtual unsigned int execute(function *fun) override {
    basic_block bb;
    std::set<tree> potential_const_args;
    std::set<tree> potential_const_pointer_args;
    std::unordered_map<tree, tree> pointer_of_ref;

    tree arg = DECL_ARGUMENTS(fun->decl);
    while (arg) {
      if (TREE_CODE(TREE_TYPE(arg)) == POINTER_TYPE)
        potential_const_pointer_args.insert(arg);
      else
        potential_const_args.insert(arg);
      arg = TREE_CHAIN(arg);
    }

    FOR_ALL_BB_FN(bb, fun) {
      gimple_stmt_iterator gsi;
      for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple *stmt = gsi_stmt(gsi);
        switch (gimple_code(stmt)) {
        case GIMPLE_ASSIGN: {
          tree lhs = gimple_assign_lhs(stmt);

          if (lhs != NULL && TREE_CODE(lhs) == PARM_DECL) {
            if (potential_const_args.find(lhs) != potential_const_args.end()) {
              potential_const_args.erase(lhs);
            }
          }

          tree rhs1 = gimple_assign_rhs1(stmt);
          tree rhs2 = gimple_assign_rhs2(stmt);
          tree rhs3 = gimple_assign_rhs3(stmt);

          if (rhs1 != NULL && potential_const_pointer_args.find(rhs1) !=
                                  potential_const_pointer_args.end())
            pointer_of_ref[lhs] = rhs1;
          if (rhs2 != NULL && potential_const_pointer_args.find(rhs2) !=
                                  potential_const_pointer_args.end())
            pointer_of_ref[lhs] = rhs2;
          if (rhs3 != NULL && potential_const_pointer_args.find(rhs3) !=
                                  potential_const_pointer_args.end())
            pointer_of_ref[lhs] = rhs3;

          if (TREE_CODE(lhs) == MEM_REF) {
            auto ref = TREE_OPERAND(lhs, 0);
            if (pointer_of_ref.find(ref) != pointer_of_ref.end())
              potential_const_pointer_args.erase(pointer_of_ref[ref]);
          }
          break;
        }
        case GIMPLE_CALL: {
          tree lhs = gimple_call_lhs(stmt);
          if (lhs != NULL && TREE_CODE(lhs) == PARM_DECL) {
            if (potential_const_args.find(lhs) != potential_const_args.end()) {
              potential_const_args.erase(lhs);
            }
          }
          if (lhs != NULL && TREE_CODE(lhs) == MEM_REF) {
            auto ref = TREE_OPERAND(lhs, 0);
            if (pointer_of_ref.find(ref) != pointer_of_ref.end())
              potential_const_pointer_args.erase(pointer_of_ref[ref]);
          }

          tree arg_type = TYPE_ARG_TYPES(gimple_call_fntype(stmt));
          unsigned nargs = gimple_call_num_args(stmt);
          for (unsigned i = 0; i < nargs; ++i) {
            tree arg = gimple_call_arg(stmt, i);
            if (arg_type) {
              tree dtype = TREE_TYPE(TREE_VALUE(arg_type));
              if (dtype && !TREE_READONLY(dtype)) {
                potential_const_pointer_args.erase(arg);
              }
              arg_type = TREE_CHAIN(arg_type);
            }
          }
          break;
        }
        default:
          break;
        }
      }
    }
    for (auto it = potential_const_args.begin();
         it != potential_const_args.end(); ++it) {
      warning_at(DECL_SOURCE_LOCATION(*it), 0, "argument can be made const");
    }
    for (auto it = potential_const_pointer_args.begin();
         it != potential_const_pointer_args.end(); ++it) {
      warning_at(DECL_SOURCE_LOCATION(*it), 0, "argument can be made const");
    }
    return 0;
  }
};

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
  if (!plugin_default_version_check(version, &gcc_version)) {
    std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR
              << "." << GCCPLUGIN_VERSION_MINOR << "\n";
    return 1;
  }

  register_callback(plugin_info->base_name,
                    /* event */ PLUGIN_INFO,
                    /* callback */ NULL, /* user_data */
                    &my_gcc_plugin_info);

  // Register the phase right after cfg
  struct register_pass_info pass_info;

  pass_info.pass = new warn_add_const(g);
  pass_info.reference_pass_name = "cfg";
  pass_info.ref_pass_instance_number = 1;
  pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL,
                    &pass_info);

  return 0;
}
