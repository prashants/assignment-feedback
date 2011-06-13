<?php
// $Id$

function add_feedback_settings_form($form_state)
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
    '#options' => array('0' => 'Please select...', '1' => 'Assignment', '2' => 'Feedback', '3' => 'Correction', '4' => 'Question', '5' => 'Suggestions', '6' => 'Others'),
    '#required' => TRUE,
  );

  $form['feedback_where'] = array(
    '#type' => 'radios',
    '#title' => t('Where'),
    '#options' => array('1' => 'Inbetween Tutorial', '2' => 'End of Tutorial'),
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

function add_feedback_settings_form_validate($form, &$form_state)
{
  if ($form_state['values']['feedback_type'] < 1)
    form_set_error('feedback_type', t('Feedback Type field is required.'));
  if (!preg_match('/^[0-9][0-9]:[0-9][0-9]$/', $form_state['values']['timing']))
    form_set_error('timing', t('Start Time is not in a valid format (eg: 00:00).'));
  if (!preg_match('/^\d+$/', $form_state['values']['completion_time']))
    form_set_error('completion_time', t('How long did you take to complete the assignment is not a valid time.'));
}

function add_feedback_settings_form_submit($form, &$form_state)
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
	drupal_set_message(t('Thank you for your feedback!'), 'status');
	drupal_goto('node/' . $nid);

	/* sending email */
	$message['to'] = $user->mail;
	$message['subject'] = t('Feedback Received');
	$message['body'] = t('We have received your feedback for ') . $node_data->title;
	if (variable_get('assignment_feedback_from_email', NULL))
		$message['headers'] = array('From' => variable_get('assignment_feedback_from_email', NULL));

	if (!drupal_mail_send($message))
		drupal_set_message('Error sending email message.', 'error');

	if (variable_get('assignment_feedback_emails', NULL)) {
		$message['to'] = variable_get('assignment_feedback_emails', '');
		if (!drupal_mail_send($message))
			drupal_set_message('Error sending email message.', 'error');
	}
}

