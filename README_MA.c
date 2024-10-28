/*
 * |-----------------------------------------------------
 * |===[ Naming Conventions ]===
 * |-----------------------------------------------------
 * |=[ Common ]=
 * | increase                    : inc
 * | decrease                    : dec
 * | l_                          : local
 * |
 * | change                      : change occasionally
 * | update                      : change frequently
 * | if                          : result in true/false
 * | check                       : result in many cases
 * |
 * |=[ Special ]=
 * | recycler                    : rc
 * | moniter actuator            : ma
 * | moniter actuator controller : mac
 * | stepper motor               : sm
 * | stepper motor controller    : smc
 * |-----------------------------------------------------
 *
 * [ process_ma ]
 *
 * process_sm1() -> check(smc.state)
 * - (running) ->
 *   if(smc.mi_up_flag, 0) (true) ->
 *    if(m_next_speed < smc.speed) (true) ->
 *      create(new_next_speed, m_next_speed + delta) ->
 *      if(new_next_speed > smc.speed) (true) ->
 *        update_smc(new_next_speed, smc.speed)
 *      prepare_sm1(new_next_speed)
 *      - dprintf(new_next_speed) ->
 *        update_smc(m_next_speed, new_next_speed) ->
 *        update_smc(m_next_pos, m_next_speed/mdt) ->
 *        update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed)) ->
 *      update_smc(mi_up_flag, 1)
 *
 *      // get_ocr_for_speed_sm1 -> F_CPU / 256 / (m_next_speed * 2)
 *
 * - (stopped) -> if(smc.mi_up_flag, 0) (true) ->
 *    create(new_next_speed, m_next_speed - delta) ->
 *    if(m_next_speed >= min) (true) ->
 *      if(new_next_speed < min)
 *      - (true) -> mi_stop -> disble(clk_src) -> cbi(TIMSK3, OCIE3A) ->
 *        change_smc(state, ready)
 *      - (false) ->
 *        prepare_sm1(new_next_speed)
 *        - dprintf(new_next_speed) ->
 *          update_smc(m_next_speed, new_next_speed) ->
 *          update_smc(m_next_pos, m_next_speed/mdt) ->
 *          update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed))
 *        update_smc(mi_up_flag, 1)
 *
 * - (moving) -> if(smc.mi_up_flag, 0) (true) ->
 *   if(m_next_speed, 0)
 *   - (true) -> dprintf(move_to_stop) -> change_smc(state, ready)
 *   - (false) -> check_smc(astate)
 *
 *    - (4) -> update_smc(mi_stop_flag, 1)
 *    - (else) ->
 *      calc(sm_next_plan) -> check_smc(astate)
 *      - (0: start) ->
 *        init(m_prev_pos, 0) ->
 *        init(m_prev_speed, min) ->
 *        if(m_next_speed >= max)
 *        - (true) ->
 *          update_smc(m_next_pos, m_pb) -> change_smc(astate, 2)
 *        - (false) ->
 *          update_smc(m_next_pos, m_next_speed/mdt) ->
 *          if(m_next_pos, 0) (true) -> update_smc(m_next_pos, 1) ->
 *          change_smc(astate, 1)
 *
 *      - (1: accel) ->
 *        inc(m_prev_pos, m_next_pos) ->
 *        inc(m_next_speed, delta) ->
 *        if(m_next_speed >= max)
 *        - (true) ->
 *          update_smc(m_next_pos, m_pb - m_prev_pos) ->
 *          if(m_next_pos, 0) (true) -> update_smc(m_next_pos, 1) ->
 *          change_smc(astate, 2)
 *
 *        - (false) ->
 *          update_smc(m_next_pos, m_next_speed/mdt) ->
 *          if(m_next_pos, 0) (true) -> update_smc(m_next_pos ,1)
 *
 *      - (2: max) ->
 *        inc(m_prev_pos, m_next_pos) ->
 *        dec(m_next_speed, delta) ->
 *        if(m_next_speed <= min)
 *        - (true) ->
 *          update_smc(m_next_speed, min) ->
 *          update_smc(m_next_pos, m_dist - m_prev_pos) -> change_smc(astate, 4)
 *        - (false) ->
 *          update_smc(m_next_pos, m_next_speed/mdt) ->
 *          if(m_next_pos, 0) (true) -> update_smc(m_next_pos, 1) ->
 *          change_smc(astate, 3)
 *
 *      - (3: de-accel) ->
 *        inc(m_prev_pos, m_next_pos) ->
 *        dec(m_next_speed, delta) ->
 *        if(m_next_speed <= min)
 *        - (true) ->
 *          update_smc(m_next_speed, min) ->
 *          update_smc(m_next_pos, m_dist - m_prev_pos) -> change_smc(astate, 4)
 *        - (false) ->
 *          update_smc(m_next_pos, m_next_speed/mdt) ->
 *          if(m_next_pos, 0) (true) -> update_smc(m_next_pos, 1) ->
 *          change_smc(astate, 3)
 *
 *      - (4) -> reset(m_prev_pos, m_next_speed)
 *
 *      update_smc(m_next_ocr, ocr_from_next_speed(m_next_speed)) ->
 *      update_smc(mi_up_flag, 1) ->
 *      if(smc.astate, 4) (true) -> update_smc(mi_stop_flag, 1)
 *
 * create(l_prev_step, mac.init_step) ->
 * check(mac.state)
 * - (init) ->
 *   check(mac, init_step)
 *   - (0: unknown) ->
 *      if(org_sensor, on)
 *      - (true: org) ->
 *        run_sm1(away_org, 400)
 *        - if(mc.state, ready) ->
 *          - (true: state) ->
 *            dprintf(sm1_run) ->
 *            change_smc(astate, 0) ->
 *            change_smc(speed, speed) ->
 *            change_smc(dir, away_org) ->
 *            create(new_next_speed, smc.min)
 *            if(new_next_speed > smc.speed) (true) ->
 *              prepare_sm1(new_next_speed)
 *              - dprintf(new_next_speed) ->
 *                update_smc(m_next_speed, new_next_speed) ->
 *                update_smc(m_next_pos, m_next_speed/mdt) ->
 *                update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed)) ->
 *            if (smc.dir, away_org) (true) -> cbi(PORTK, 6)
 *            enable(sm) -> sbi(PORTK, 3)
 *            mi_start()
 *            - update_smc(mi_dpos, 0) ->
 *              update_smc(mi_next_pos, smc.m_next_pos) ->
 *              change(OCR3A, smc.m_next_ocr) ->
 *              update_smc(mi_up_flag, 0) ->
 *              update_smc(mi_stop_flag, 0) ->
 *              change(TCNT3H, 0) -> change(TCNT3L, 0) ->
 *              change(TCNT3A, 0x40) -> change(TCNT3B, 0x0C) ->
 *              sbi(TIMSK3, OCIE3A)
 *              update_smc(state, running) ->
 *            return(0): success
 *          - (false: state) -> return(-1): err
 *        update_mac(init_step, 10)
 *      - (false: org)
 *        run_sm1(to_org, 400)
 *        - if(mc.state, ready) ->
 *          - (true: state) ->
 *            dprintf(sm1_run) ->
 *            change_smc(astate, 0) ->
 *            change_smc(speed, speed) ->
 *            change_smc(dir, to_org) ->
 *            create(new_next_speed, smc.min)
 *            if(new_next_speed > smc.speed) (true) ->
 *              prepare_sm1(new_next_speed) ->
 *              - dprintf(new_next_speed) ->
 *                update_smc(m_next_speed, new_next_speed) ->
 *                update_smc(m_next_pos, m_next_speed/mdt) ->
 *                update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed))
 *            if (smc.dir, away_org) (false) -> sbi(PORTK, 6)
 *            enable(sm) -> sbi(PORTK, 3)
 *            mi_start()
 *            - update_smc(mi_dpos, 0) ->
 *              update_smc(mi_next_pos, smc.m_next_pos) ->
 *              change(OCR3A, smc.m_next_ocr) ->
 *              update_smc(mi_up_flag, 0) ->
 *              update_smc(mi_stop_flag, 0) ->
 *              change(TCNT3H, 0) -> change(TCNT3L, 0) ->
 *              change(TCNT3A, 0x40) -> change(TCNT3B, 0x0C) ->
 *              sbi(TIMSK3, OCIE3A)
 *              update_smc(state, running) ->
 *            return(0): success
 *          - (false: state) -> return(-1): err
 *        update_mac(init_step, 1)
 *
 *   - (1: find unknown) ->
 *      if(org_sensor, on) (true) -> stop_sm1()
 *      - check(smc.state)
 *        - (moving) -> change_smc(state, ready) ->
 *          stop_mi()
 *          - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *        - (running) -> change_smc(state, ready) ->
 *          stop_mi()
 *          - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *        dprintf(sm1_stop)
 *      set_timer(tid_op, 100)
 *      - if(t, invalid) (true) -> return(-1): err
 *        if(t_flag, alloc)
 *        - (true) ->
 *          change(t_value, 100) -> change(t_flag, alloc_run) ->
 *          return(0): success
 *        - (false) -> return(-1): err
 *      change_mac(init_step, 2)
 *
 *   - (2: find unknown) ->
 *      if(t, fired) (true) ->
 *      clear_timer()
 *      - if(t, invalid) (true) -> return(-1): err
 *        if(t, alloc)
 *        - (true) -> change(t_flag, alloc) -> return(-1): err
 *        - (false) -> return(0): success
 *      run_sm1(away_org, 400)
 *      - if(mc.state, ready) ->
 *          - (true: state) ->
 *            dprintf(sm1_run) ->
 *            change_smc(astate, 0) ->
 *            change_smc(speed, speed) ->
 *            change_smc(dir, away_org) ->
 *            create(new_next_speed, smc.min)
 *            if(new_next_speed > smc.speed) (true) ->
 *              prepare_sm1(new_next_speed)
 *              - dprintf(new_next_speed) ->
 *                update_smc(m_next_speed, new_next_speed) ->
 *                update_smc(m_next_pos, m_next_speed/mdt) ->
 *                update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed)) ->
 *            if (smc.dir, away_org) (true) -> cbi(PORTK, 6)
 *            enable(sm) -> sbi(PORTK, 3)
 *            mi_start()
 *            - update_smc(mi_dpos, 0) ->
 *              update_smc(mi_next_pos, smc.m_next_pos) ->
 *              change(OCR3A, smc.m_next_ocr) ->
 *              update_smc(mi_up_flag, 0) ->
 *              update_smc(mi_stop_flag, 0) ->
 *              change(TCNT3H, 0) -> change(TCNT3L, 0) ->
 *              change(TCNT3A, 0x40) -> change(TCNT3B, 0x0C) ->
 *              sbi(TIMSK3, OCIE3A)
 *              update_smc(state, running) ->
 *            return(0): success
 *          - (false: state) -> return(-1): err
 *      change_mac(init_step, 10)
 *
 *   - (10: out_of_init_pos) ->
 *      if(org_sensor, on) (true) ->
 *      stop_sm1()
 *      - check(smc.state)
 *        - (moving) -> change_smc(state, ready) ->
 *          stop_mi()
 *          - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *        - (running) -> change_smc(state, ready) ->
 *          stop_mi()
 *          - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *        dprintf(sm1_stop)
 *      set_timer(tid_op, 100)
 *      - if(t, invalid) (true) -> return(-1): err
 *        if(t_flag, alloc)
 *        - (true) ->
 *          change(t_value, 100) -> change(t_flag, alloc_run) ->
 *          return(0): success
 *        - (false) -> return(-1): err
 *      change_mac(init_step, 11)
 *
 *   - (11: find_limit_again) ->
 *      if(t, fired) (true) ->
 *      clear_timer()
 *      - if(t, invalid) (true) -> return(-1): err
 *        if(t, alloc)
 *        - (true) -> change(t_flag, alloc) -> return(-1): err
 *        - (false) -> return(0): success
 *      run_sm1(to_org, 400)
 *      - if(mc.state, ready) ->
 *          - (true: state) ->
 *            dprintf(sm1_run) ->
 *            change_smc(astate, 0) ->
 *            change_smc(speed, speed) ->
 *            change_smc(dir, to_org) ->
 *            create(new_next_speed, smc.min)
 *            if(new_next_speed > smc.speed) (true) ->
 *              prepare_sm1(new_next_speed)
 *              - dprintf(new_next_speed) ->
 *                update_smc(m_next_speed, new_next_speed) ->
 *                update_smc(m_next_pos, m_next_speed/mdt) ->
 *                update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed)) ->
 *            if (smc.dir, away_org) (false) -> sbi(PORTK, 6)
 *            enable(sm) -> sbi(PORTK, 3)
 *            mi_start()
 *            - update_smc(mi_dpos, 0) ->
 *              update_smc(mi_next_pos, smc.m_next_pos) ->
 *              change(OCR3A, smc.m_next_ocr) ->
 *              update_smc(mi_up_flag, 0) ->
 *              update_smc(mi_stop_flag, 0) ->
 *              change(TCNT3H, 0) -> change(TCNT3L, 0) ->
 *              change(TCNT3A, 0x40) -> change(TCNT3B, 0x0C) ->
 *              sbi(TIMSK3, OCIE3A)
 *              update_smc(state, running) ->
 *            return(0): success
 *          - (false: state) -> return(-1): err
 *      change_mac(init_step, 20)
 *
 *   - (20: init_pos) ->
 *      if(org_sensor, on) (true) ->
 *        stop_sm1()
 *        - check(smc.state)
 *          - (moving) -> change_smc(state, ready) ->
 *            stop_mi()
 *            - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *          - (running) -> change_smc(state, ready) ->
 *            stop_mi()
 *            - change(TCCR3B, 0x08) -> cbi(TIMSK3, OCIE3A)
 *          dprintf(sm1_stop)
 *        reset_mac(init_step, 0) ->
 *        reset_mac(position, 0) ->
 *        reset_mac(state, ready) ->
 *        reset_pos_sm1()
 *        - if(smc.state, ready)
 *          - (true) -> update_smc(cp_pos, 0) -> return(0): success
 *          - (false) -> return(-1): err
 *
 *      if(t, fired) (true) ->
 *        clear_timer()
 *        - if(t, invalid) (true) -> return(-1): err
 *          if(t, alloc)
 *          - (true) -> change(t_flag, alloc) -> return(-1): err
 *          - (false) -> return(0): success
 *        run_sm1(to_org, 400)
 *        - if(mc.state, ready) ->
 *          - (true: state) ->
 *            dprintf(sm1_run) ->
 *            change_smc(astate, 0) ->
 *            change_smc(speed, speed) ->
 *            change_smc(dir, to_org) ->
 *            create(new_next_speed, smc.min)
 *            if(new_next_speed > smc.speed) (true) ->
 *              prepare_sm1(new_next_speed)
 *              - dprintf(new_next_speed) ->
 *                update_smc(m_next_speed, new_next_speed) ->
 *                update_smc(m_next_pos, m_next_speed/mdt) ->
 *                update_smc(m_next_ocr, get_ocr_for_speed_sm1(m_next_speed))
 *            if (smc.dir, away_org) (false) -> sbi(PORTK, 6)
 *            enable(sm) -> sbi(PORTK, 3)
 *            mi_start()
 *            - update_smc(mi_dpos, 0) ->
 *              update_smc(mi_next_pos, smc.m_next_pos) ->
 *              change(OCR3A, smc.m_next_ocr) ->
 *              update_smc(mi_up_flag, 0) ->
 *              update_smc(mi_stop_flag, 0) ->
 *              change(TCNT3H, 0) -> change(TCNT3L, 0) ->
 *              change(TCNT3A, 0x40) -> change(TCNT3B, 0x0C) ->
 *              sbi(TIMSK3, OCIE3A)
 *              update_smc(state, running) ->
 *            return(0): success
 *          - (false: state) -> return(-1): err
 *        change_mac(init_step, 20)
 *
 * - (moving) ->
 *   get_state_sm1(l_state)
 *   get_pos_sm1(l_pos)
 *   if (l_state, ready) (true) ->
 *      dprintf(moving -> ready)
 *      change_mac(state, ready)
 *      change_mac(pos, l_pos/pos2step)
 *
 * if (l_prev_step != mac.init_step) (true) ->
 *    dprintf(prev_step -> init_step)
 *
 * []
 * */
