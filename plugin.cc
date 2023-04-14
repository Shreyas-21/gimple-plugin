#include <iostream>
#include <set>
#include <unordered_map>

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
#include "tree-pass.h"
#include "tree.h"
// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = {
    "1.0", "This plugin emits warn_dead_code for C++"};

const pass_data warn_add_const_data = {
    GIMPLE_PASS,
    "warn_unused_result_cxx", /* name */
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

    tree arg = DECL_ARGUMENTS(fun->decl);
    while (arg) {
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
            if (potential_const_args.find(lhs) !=
                potential_const_args.end()) {
              potential_const_args.erase(lhs);
            }
          }
          break;
        }
        case GIMPLE_CALL: {
          tree lhs = gimple_call_lhs(stmt);
          if (lhs != NULL && TREE_CODE(lhs) == PARM_DECL) {
            if (potential_const_args.find(lhs) !=
                potential_const_args.end()) {
              potential_const_args.erase(lhs);
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
