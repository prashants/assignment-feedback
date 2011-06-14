<?php
// $Id$

function manage_feedback_form($form_state)
{
	global $user;

	$feedback_id = (int)arg(3);
	$feedback_q = db_query("SELECT * FROM assignment_feedback_submissions WHERE id = %d LIMIT 1", $feedback_id);
	if (!$feedback_q) {
		drupal_set_message(t('Invalid Feedback'), 'error');
		drupal_goto('feedback/manage');
	}
	$feedback_data = db_fetch_object($feedback_q);
	$user_data = user_load($feedback_data->uid);
	$node_data = node_load($feedback_data->nid);

  $form['name'] = array(
    '#type' => 'item',
    '#title' => t('Your username'),
    '#value' => $user_data->name,
  );

  $form['tutorial_name'] = array(
    '#type' => 'item',
    '#title' => t('Name of the Tutorial'),
    '#value' => $node_data->title,
  );

  $form['feedback_type'] = array(
    '#type' => 'item',
    '#title' => t('Feedback Type'),
    '#value' => _feedback_type($feedback_data->feedback_type),
  );

  $form['feedback_where'] = array(
    '#type' => 'item',
    '#title' => t('Where'),
    '#value' => _feedback_where($feedback_data->feedback_where),
  );

  $form['timing'] = array(
    '#type' => 'item',
    '#title' => t('Start Time'),
    '#value' => $feedback_data->timing,
  );

  $form['completion_time'] = array(
    '#type' => 'item',
    '#title' => t('How long did you take to complete the assignment'),
    '#value' => $feedback_data->completion_time . ' minutes',
  );

  $form['comments'] = array(
    '#type' => 'item',
    '#title' => t('Comment Box'),
    '#value' => $feedback_data->comments,
  );

  $form['status'] = array(
    '#type' => 'radios',
    '#title' => t('Approval Status'),
    '#options' => array(1 => 'Approve', 2 => 'Disapprove'),
    '#required' => TRUE,
  );

  $form['message'] = array(
    '#type' => 'textarea',
    '#title' => t('Reason for disapproval'),
  );

  $form['feedback_id'] = array(
    '#type' => 'hidden',
    '#value' => $feedback_id,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );

  $form['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('Cancel'), 'feedback/manage'),
  );

  return $form;
}

function manage_feedback_form_submit($form, &$form_state)
{
	global $user;
	$nid = (int)arg(2);
	$node_data = node_load($nid);
	if (!$node_data) {
		drupal_set_message(t('Invalid Link'), 'error');
		drupal_goto('');
	}

	db_query("INSERT INTO {assignment_feedback_submissions} (uid, nid, feedback_type, feedback_where, timing, completion_time, comments, status, timestamp)
						VALUES (%d, %d, %d, %d, '%s', %d, '%s', %d, %d)",
						$user->uid,
						$nid,
						$form_state['values']['feedback_type'],
						$form_state['values']['feedback_where'],
						$form_state['values']['timing'],
						$form_state['values']['completion_time'],
						$form_state['values']['comments'],
						0,
						time()
						);

	/* sending email */
	$email_to = $user->mail;
	$param['feedback_received']['user_id'] = $user->uid;
	$param['feedback_received']['tutorial_name'] = $node_data->title;
	$param['feedback_received']['feedback_type'] = _feedback_type($form_state['values']['feedback_type']); 
	$param['feedback_received']['feedback_where'] = _feedback_where($form_state['values']['feedback_where']);
	$param['feedback_received']['timing'] = $form_state['values']['timing'];
	$param['feedback_received']['completion_time'] = $form_state['values']['completion_time'];
	$param['feedback_received']['comments'] = $form_state['values']['comments'];
	if (!drupal_mail('assignment_feedback', 'feedback_received', $email_to, language_default(), $param, variable_get('textbook_companion_from_email', NULL), TRUE))
		drupal_set_message('Error sending email message.', 'error');
	$email_to = variable_get('assignment_feedback_emails', NULL);
	if (!drupal_mail('assignment_feedback', 'feedback_received', $email_to, language_default(), $param, variable_get('textbook_companion_from_email', NULL), TRUE))
		drupal_set_message('Error sending email message.', 'error');

	drupal_set_message(t('Thank you for your feedback!'), 'status');
	drupal_goto('node/' . $nid);
}


function edit_feedback_form($form_state)
{
	global $user;

	$feedback_id = (int)arg(3);
	$feedback_q = db_query("SELECT * FROM assignment_feedback_submissions WHERE id = %d LIMIT 1", $feedback_id);
	if (!$feedback_q) {
		drupal_set_message(t('Invalid Feedback'), 'error');
		drupal_goto('feedback/manage');
	}
	$feedback_data = db_fetch_object($feedback_q);
	$user_data = user_load($feedback_data->uid);
	$node_data = node_load($feedback_data->nid);

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Your username'),
    '#size' => 30,
    '#value' => $user_data->name,
    '#disabled' => TRUE,
  );

  $form['tutorial_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of the Tutorial'),
    '#size' => 30,
    '#value' => $node_data->title,
    '#disabled' => TRUE,
  );

  $form['feedback_type'] = array(
    '#type' => 'select',
    '#title' => t('Feedback Type'),
    '#default_value' => $feedback_data->feedback_type,
    '#options' => _feedback_type(),
    '#required' => TRUE,
  );

  $form['feedback_where'] = array(
    '#type' => 'radios',
    '#title' => t('Where'),
    '#default_value' => $feedback_data->feedback_where,
    '#options' => _feedback_where(),
    '#required' => TRUE,
  );

  $form['timing'] = array(
    '#type' => 'textfield',
    '#title' => t('Start Time'),
    '#description' => t('Example : 02:40 for 2 minutes 40 seconds from the start of tutorial'),
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => $feedback_data->timming,
    '#required' => TRUE,
  );

  $form['completion_time'] = array(
    '#type' => 'textfield',
    '#title' => t('How long did you take to complete the assignment'),
    '#size' => 2,
    '#maxlength' => 2,
    '#field_suffix' => 'minutes',
    '#default_value' => $feedback_data->completion_time,
    '#required' => TRUE,
  );

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Comment Box'),
    '#description' => t('Enter your comments, questions or suggestions here'),
    '#rows' => 12,
    '#default_value' => $feedback_data->comments,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );

  $form['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('Cancel'), 'manage_proposal'),
  );

  return $form;
}

function _feedback_type($type_id = NULL)
{
	if ($type_id) {
		switch ($type_id) {
		case 0: return 'Invalid'; break;
		case 1: return 'Assignment'; break;
		case 2: return 'Feedback'; break;
		case 3: return 'Correction'; break;
		case 4: return 'Question'; break;
		case 5: return 'Suggestions'; break;
		case 6: return 'Others'; break;
		default: return 'Invalid'; break;
		}
	} else {
		return array('0' => 'Please select...', '1' => 'Assignment', '2' => 'Feedback', '3' => 'Correction', '4' => 'Question', '5' => 'Suggestions', '6' => 'Others');
	}
}

function _feedback_where($type_id = NULL)
{
	if ($type_id) {
		switch ($type_id) {
		case 0: return 'Invalid'; break;
		case 1: return 'Inbetween Tutorial'; break;
		case 2: return 'End of Tutorial'; break;
		default: return 'Invalid'; break;
		}
	} else {
		return array('1' => 'Inbetween Tutorial', '2' => 'End of Tutorial');
	}
}

function _pending_feedbacks()
{
	$pending_q = db_query("SELECT * FROM {assignment_feedback_submissions} WHERE status = 0 ORDER BY id DESC");
	if (!$pending_q) {
		drupal_set_message(t('There are no pending feedback.'), 'status');
		return '';
	}
	$pending_rows = array();
	while ($pending_data = db_fetch_object($pending_q)) {
		$pending_rows[] = array(
				date('d-m-Y', $pending_data->timestamp),
				l(node_load($pending_data->nid)->title, 'node/' . $pending_data->nid),
				l(user_load($pending_data->uid)->name, 'user/' . $pending_data->uid),
				l('Edit', 'feedback/manage/approve/' . $pending_data->id)
			);
	}
  $pending_header = array('Date of Submission', 'Name of the Tutorial', 'User Name', 'Action');
  $output = theme_table($pending_header, $pending_rows);
  return $output; 
}

function _all_feedbacks()
{
	return '';
}
