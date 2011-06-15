<?php
// $Id$

function add_feedback_form($form_state)
{
	global $user;

	$nid = (int)arg(2);
	$node_data = node_load($nid);
	if (!$node_data) {
		drupal_set_message(t('Invalid Link'), 'error');
		drupal_goto('');
	}
	
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Your username'),
    '#size' => 30,
    '#value' => $user->name,
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
    '#default_value' => 0,
    '#options' => _feedback_type(),
    '#required' => TRUE,
  );

  $form['feedback_where'] = array(
    '#type' => 'radios',
    '#title' => t('Where'),
    '#default_value' => 1,
    '#options' => _feedback_where(),
    '#required' => TRUE,
  );

  $form['timing'] = array(
    '#type' => 'textfield',
    '#title' => t('Start Time'),
    '#description' => t('Example : 02:40 for 2 minutes 40 seconds from the start of tutorial'),
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => '00:00',
    '#required' => TRUE,
  );

  $form['completion_time'] = array(
    '#type' => 'textfield',
    '#title' => t('How long did you take to complete the assignment'),
    '#size' => 2,
    '#maxlength' => 2,
    '#field_suffix' => 'minutes',
    '#required' => TRUE,
  );

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Comment Box'),
    '#description' => t('Enter your comments, questions or suggestions here'),
    '#rows' => 12,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit Feedback'),
  );

  return $form;
}

function add_feedback_form_validate($form, &$form_state)
{
  if ($form_state['values']['feedback_type'] < 1)
    form_set_error('feedback_type', t('Feedback Type field is required.'));
  if (!preg_match('/^[0-9][0-9]:[0-9][0-9]$/', $form_state['values']['timing']))
    form_set_error('timing', t('Start Time is not in a valid format (eg: 00:00).'));
  if (!preg_match('/^\d+$/', $form_state['values']['completion_time']))
    form_set_error('completion_time', t('How long did you take to complete the assignment is not a valid time.'));
}

function add_feedback_form_submit($form, &$form_state)
{
	global $user;
	$nid = (int)arg(2);
	$node_data = node_load($nid);
	if (!$node_data) {
		drupal_set_message(t('Invalid Link'), 'error');
		drupal_goto('');
	}

	db_query("INSERT INTO {assignment_feedback_submissions} (uid, nid, feedback_type, feedback_where, timing, completion_time, comments, status, timestamp, approver_uid, approver_timestamp)
						VALUES (%d, %d, %d, %d, '%s', %d, '%s', %d, %d, %d, %d)",
						$user->uid,
						$nid,
						$form_state['values']['feedback_type'],
						$form_state['values']['feedback_where'],
						$form_state['values']['timing'],
						$form_state['values']['completion_time'],
						$form_state['values']['comments'],
						0,
						time(),
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
